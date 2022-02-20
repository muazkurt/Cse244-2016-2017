#include "Listdir.h"

#include "restart.h"

#ifndef __USE_MISC
	#define __USE_MISC
#endif
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define MAX_THRADS	   32
#define MAX_FILE_COUNT 16

#define PARENT_READ	 0
#define PARENT_WRITE 1
#define CHILD_READ	 0
#define CHILD_WRITE	 1
#define FALSE		 0
#define TRUE		 1

#define LOG_FILENAME "log.log"
#include <types.h>
static pthread_once_t lockerror = PTHREAD_ONCE_INIT;

main_return_val collectFifo(const char *fifoPath) {
	main_return_val collect	 = {0};
	int				collects = 0, fifoDescriptor = -1;
	if (FAILURE_CASE == (fifoDescriptor = r_open2(fifoPath, O_RDONLY)))
		fprintf(stderr, "open error (%s), errno(%d)\n", fifoPath, errno);
	else {
		while (wait(NULL) > 0) continue;
		for (main_return_val subdir_info = {0};
			 r_read(fifoDescriptor, &subdir_info, sizeof(main_return_val)) > 0;) {
			collect.error_detect += subdir_info.error_detect;
			collect.subdir_members_main += subdir_info.subdir_members_main;
			collect.total_files_main += subdir_info.total_files_main;
			collect.total_line_count += subdir_info.total_line_count;
			collect.total_occurences += subdir_info.total_occurences;
			collect.total_threads_main += subdir_info.total_threads_main;
		}
		if (FAILURE_CASE == r_close(fifoDescriptor))
			fprintf(stderr, "close error, errno(%d)\n", errno);
	}
	return collect;
}

main_return_val *listDir_r(const parameters *request) {
	int returnVal = FAILURE_CASE, fifoCreated = FAILURE_CASE, has_subdir = FALSE,
		fifoDescriptor = FAILURE_CASE, find_count = 0;
	char			 fifoFilename[ PATH_MAX ] = {0}, path[ PATH_MAX ] = {0};
	pthread_t		 thread_family[ MAX_THRADS ];
	main_return_val *dir_info;
	DIR				*openedDir	= NULL;
	struct dirent	  *dirContent = NULL;

	snprintf(path, PATH_MAX, "%s%s", request->base, request->ref);
	snprintf(fifoFilename, PATH_MAX, "%s.fifo", path);
	dir_info = (main_return_val *) calloc(1, sizeof(main_return_val));
	if (FAILURE_CASE == (returnVal = isdirectory(path))) {
		fprintf(stderr, "%s is not a directory\n", path);
		++dir_info->error_detect;
		returnVal = FAILURE_CASE;
	} else if (NULL == (openedDir = opendir(path))) {
		fprintf(stderr, "Failed while opening directory %s\n", path);
		++dir_info->error_detect;
		returnVal = FAILURE_CASE;
	} else if (FAILURE_CASE == (fifoCreated = r_mkfifo(fifoFilename, FIFO_PERM))) {
		fprintf(stderr, "mkfifo error (%s), errno(%d)\n", fifoFilename, errno);
		++dir_info->error_detect;
		returnVal = FAILURE_CASE;
	} else
		while (NULL != (dirContent = readdir(openedDir))) {
			if (0 == strncmp(dirContent->d_name, LOG_FILENAME, sizeof(LOG_FILENAME) - 1)
				|| 0 == strcmp(dirContent->d_name, ".") || 0 == strcmp(dirContent->d_name, "..")) {
				++dir_info->error_detect;
			} else {
				parameters p = {
					.counter_sem = request->counter_sem,
					.base		 = request->base,
					.query		 = request->query,
					.log_sem	 = request->log_sem,
					.ref		 = {0},
				};
				snprintf(p.ref, PATH_MAX, "%s/%s", request->ref, dirContent->d_name);
				switch (dirContent->d_type) {
					case DT_DIR: // Directory
						has_subdir		 = TRUE;
						int childProcess = FAILURE_CASE;
						if (FAILURE_CASE == (childProcess = fork())) {
							fprintf(stderr,
									"Failed while fork for(%s), errno: (%d)\n",
									dirContent->d_name,
									errno);
						} else if (0 == childProcess) {
							main_return_val *subdir_info = listDir_r(&p);
							if (NULL == subdir_info) {
								fprintf(stderr,
										"Error: pid(%d), path(%s/%s).\n",
										getpid(),
										path,
										dirContent->d_name);
								exit(FAILURE_CASE);
							} else if (FAILURE_CASE
									   == (fifoDescriptor = r_open2(fifoFilename, O_WRONLY))) {
								fprintf(stderr,
										"open error (%s), errno(%d)\n",
										fifoFilename,
										errno);
								returnVal = FAILURE_CASE;
							} else {
								r_write(fifoDescriptor, subdir_info, sizeof(main_return_val));
								free(subdir_info);
								if (FAILURE_CASE == r_close(fifoDescriptor))
									fprintf(stderr, "close error, errno(%d)\n", errno);
							}
							exit(EXIT_SUCCESS);
						}
						break;
					case DT_REG: // File
						pthread_create(&thread_family[ dir_info->total_files_main++ ],
									   NULL,
									   searchFile,
									   &p);
						break;
					default:
						fprintf(stderr, "Unsupported file type. %s\n", request->ref);
						++dir_info->error_detect;
						break;
				}
			}
		}

	for (size_t i = 0; i < dir_info->total_files_main; ++i) {
		int			  tmp_int = -1;
		return_value *file_info;
		pthread_join(thread_family[ i ], (void **) &file_info);
		dir_info->total_occurences += file_info->occurences;
		dir_info->total_line_count += file_info->line_count;
		if ((tmp_int = file_info->paralel_threads) > dir_info->total_threads_main)
			dir_info->total_threads_main = tmp_int;
		free(file_info);
	}
	if (has_subdir && FAILURE_CASE != fifoCreated) {
		main_return_val collects = collectFifo(fifoFilename);
		dir_info->error_detect += collects.error_detect;
		dir_info->subdir_members_main += collects.subdir_members_main;
		dir_info->total_files_main += collects.total_files_main;
		dir_info->total_line_count += collects.total_line_count;
		dir_info->total_occurences += collects.total_occurences;
		dir_info->total_threads_main += collects.total_threads_main;
	}
	if (FAILURE_CASE != fifoCreated && FAILURE_CASE == (fifoCreated = unlink(fifoFilename))) {
		fprintf(stderr, "unlink error, errno(%d)\n", errno);
		++dir_info->error_detect;
		returnVal = FAILURE_CASE;
	}

	if (NULL != openedDir && FAILURE_CASE == (fifoCreated = closedir(openedDir)))
		fprintf(stderr, "Failed while closing the directory %s", request->base);

	if (FAILURE_CASE != returnVal) { returnVal = find_count; }
	return dir_info;
}

int init(parameters *param) {
	int retVal = 0;

	if (0 != (retVal = pthread_rwlock_init(&param->logging, NULL))) {
		fprintf(stderr, "Failed while pthread_rwlock_init (%d), error: %d", getpid(), retVal);
	} else if (SEM_FAILED
			   == (param->log_sem = sem_open(SEM_NAME_LOG, O_CREAT, (mode_t) LOG_PERMS, 1))) {
		fprintf(stderr, "Failed while creating log semaphore (%d)", getpid());
		retVal = -1;
		pthread_rwlock_destroy(&param->logging);
	} else if (SEM_FAILED
			   == (param->counter_sem = sem_open(SEM_NAME_COUNT, O_CREAT, (mode_t) LOG_PERMS, 1))) {
		fprintf(stderr, "Failed while creating counter semaphore (%d)", getpid());
		retVal = -1;
		sem_destroy(param->log_sem);
		pthread_rwlock_destroy(&param->logging);
	} else if (FAILURE_CASE
			   == (param->logFile
				   = r_open2("log.log", O_CREAT | O_APPEND | O_WRONLY | LOG_PERMS))) {
		fprintf(stderr, "Open %s is failed (pid: %d)", "log.log", getpid());
		retVal = -1;
		sem_destroy(param->counter_sem);
		sem_destroy(param->log_sem);
		pthread_rwlock_destroy(&param->logging);
	}
	return retVal;
}

int destroy(parameters *param) {
	r_close(param->logFile);
	sem_destroy(param->counter_sem);
	sem_destroy(param->log_sem);
	pthread_rwlock_destroy(&param->logging);
}
int listDir(const char *query, const char *path) {
	parameters p = {
		.base		 = path,
		.query		 = query,
		.ref		 = {0},
		.logFile	 = FAILURE_CASE,
		.log_sem	 = NULL,
		.counter_sem = NULL,
	};
	init(&p);
	listDir_r(&p);
	destroy(&p);
}