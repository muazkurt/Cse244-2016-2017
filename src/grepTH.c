#include "grepTH.h"

#include "List.h"

#include <types.h>

extern int				logfile;
static sig_atomic_t		currentworking = 0;
static sig_atomic_t		maxworker	   = 0;
static pthread_once_t	lockerror	   = PTHREAD_ONCE_INIT;
static pthread_rwlock_t logging;

static void init_rwlcog(void) { lockerror = pthread_rwlock_init(&logging, NULL); }

int init_lock_once(void) {
	if (pthread_once(&lockerror, init_rwlcog)) lockerror = EINVAL;
	return lockerror;
}

int search_pid(pid_t *stack, pid_t child_id, int stack_size) {
	int i = 0;
	for (i = 0; i < stack_size; ++i) {
		if (stack[ i ] == child_id) return i;
	}
	return -1;
}

int create_logfile(void) {
	while (open("log.log", O_CREAT | O_EXCL, LOG_PERMS) <= 0) {
		if (errno == EEXIST) {
			if (unlink("log.log") == -1) perror("Failed to refresh log");
		} else {
			perror("Failed to create_logfile");
			return -1;
		}
	}
	return 0;
}

int open_logfile(void) {
	if ((logfile = open("log.log", LOG_MODES)) <= 0) {
		perror("Failed to open_logfile");
		return -1;
	}
	return 0;
}

int log_all(char **index, int stack, sem_t *locker) {
	while (sem_wait(locker) == -1) continue;
	int i = 0, j = 0;
	while (pthread_rwlock_wrlock(&logging) != 0) continue;
	for (i = 0; i < stack; ++i)
		for (j = 0; index[ i ][ j ] != 0; ++j)
			if (write(logfile, &index[ i ][ j ], sizeof(index[ i ][ j ])) <= 0) return -1;
	while (pthread_rwlock_unlock(&logging) != 0) continue;

	while (sem_post(locker) == -1) continue;
	return 0;
}

int increaseWorkingCount(sem_t *locker) {
	int paralel_counter = 0;
	while (sem_wait(locker) == -1) continue;
	paralel_counter = ++currentworking;
	if (paralel_counter > maxworker) maxworker = paralel_counter;
	while (sem_post(locker) == -1) continue;
	return paralel_counter;
}

int decreaseWorkingCount(sem_t *locker) {
	int retVal = -1;
	while (sem_wait(locker) == -1) continue;
	retVal = --currentworking;
	while (sem_post(locker) == -1) continue;
	return retVal;
}
/// old
void *search_file_function(void *args) {
	int			  free_count = 0, find_counter = -1;
	search_file * request	= (search_file *) args;
	return_value *file_info = NULL;
	if (NULL == request) {
		fprintf(stderr, "Request is NULL (pid: %d)", getpid());
		return NULL;
	}

	// Semaphore
	file_info = searchFile(&request->param);
	// semaphore
	if (log_all(logging_index, file_info->total_string, request->log_sem) != 0)
		fprintf(stderr,
				"Failed to log all. (pid: %d, file %s/%s)",
				getpid(),
				request->param.base,
				request->param.ref);

	decreaseWorkingCount(request->counter_sem);
	// exit
	return (void *) file_info;
}

int init(sem_t **logger, sem_t **counter) {
	int retVal = 0;
	if (SEM_FAILED == (*logger = sem_open(SEM_NAME_LOG, O_CREAT, (mode_t) LOG_PERMS, 1))) {
		fprintf(stderr, "Failed while creating log semaphore (%d)", getpid());
		retVal = -1;
	}

	if (0 == retVal
		&& SEM_FAILED == (*counter = sem_open(SEM_NAME_COUNT, O_CREAT, (mode_t) LOG_PERMS, 1))) {
		fprintf(stderr, "Failed while creating counter semaphore (%d)", getpid());
		retVal = -1;
		sem_destroy(*logger);
	}
	return retVal;
}

main_return_val *ListDirfunction(const char *input_string, const char *dir_name) {
	struct dirent *dir_index  = NULL;
	DIR *		   search_dir = NULL;
	int family_members = 0, subdir_members = 0, join_count = 0, total_lines = 0, total_string = 0,
		subdir_fifo_fp[ MAX_SUB_DIR ], stack_counter = 0, maxthread, tmp_int = 0;
	char			 address[ PATH_MAX ], subdir_fifo[ MAX_SUB_DIR ][ PATH_MAX ];
	pthread_t		 thread_family[ MAX_THRADS ];
	pid_t			 subdir[ MAX_SUB_DIR ], tmp_pid;
	return_value *	 file_info;
	main_return_val *dir_info = (main_return_val *) calloc(1, sizeof(main_return_val));
	sem_t *			 sem_logger, *sem_counter;

	// semaphore
	init(&sem_logger, &sem_counter);

	while ((dir_index = readdir(search_dir)) != NULL) {
		sprintf(address, "%s/%s", dir_name, dir_index->d_name);
		// File ise
		if (isRegularFile(address)) {
			if ((strncmp(dir_index->d_name, "log.log", sizeof("log.log") - 1) == 0)
				|| (strncmp(dir_index->d_name, "fuck.c", sizeof("fuck.c") - 1) == 0)) {
			} else
			// file arama işlemleri
			{
				search_file arg = {
					.counter_sem = sem_counter,
					.log_sem = sem_logger,
					.param = {
						.base = dir_name,
						.query = input_string,
						.ref = dir_index->d_name,
					},
				};
				pthread_create(&thread_family[ join_count ], NULL, search_file_function, &args);
				++family_members;
			}
		}
		// Directory ise.
		else if (isdirectory(address)) {
			if ((strcmp(dir_index->d_name, ".") == 0) || (strcmp(dir_index->d_name, "..") == 0)) {
			} else {
				sprintf(subdir_fifo[ subdir_members ], "%d", tmp_int);
				while (mkfifo(subdir_fifo[ subdir_members ], S_IRWXU | S_IRGRP | S_IROTH) == -1) {
					if (errno == EEXIST) {
						++tmp_int;
						sprintf(subdir_fifo[ subdir_members ], "%d", tmp_int);
					}
				}
				subdir_fifo_fp[ subdir_members ] = open(subdir_fifo[ subdir_members ], O_RDWR);
				if ((subdir[ subdir_members ] = fork()) == -1)
					perror("Failed to fork for Directory!");
				if (subdir[ subdir_members ] == 0) {
					dir_info = ListDirfunction(input_string, address);
					write(subdir_fifo_fp[ subdir_members ], dir_info, sizeof(main_return_val));
					close(subdir_fifo_fp[ subdir_members ]);
					free(dir_info);
					exit(1);
				}
				++subdir_members;
			}
		}
	}

	dir_info->subdir_members_main += subdir_members;
	dir_info->total_files_main += family_members;

	while (join_count < family_members) {
		pthread_create(&thread_family[ join_count ],
					   NULL,
					   search_file_function,
					   &args;
		++join_count;
	}
	join_count = 0;

	while (join_count < family_members) {
		pthread_join(thread_family[ join_count ], (void **) &file_info);
		//		pthread_detach(thread_family[join_count]);
		++join_count;
		dir_info->total_string_main += file_info->total_string;
		dir_info->total_lines_main += file_info->total_lines;
		if ((tmp_int = file_info->paralel_threads) > dir_info->total_threads_main)
			dir_info->total_threads_main = tmp_int;
	}
	join_count = 0;

	if (subdir_members > 0) {
		while (wait(NULL) > 0) {}

		while (stack_counter < subdir_members) {
			subdir[ stack_counter ] = 0;
			read(subdir_fifo_fp[ stack_counter ], &tmp_int, sizeof(int));
			dir_info->total_string_main += tmp_int;
			read(subdir_fifo_fp[ stack_counter ], &tmp_int, sizeof(int));
			dir_info->subdir_members_main += tmp_int;
			read(subdir_fifo_fp[ stack_counter ], &tmp_int, sizeof(int));
			dir_info->total_files_main += tmp_int;
			read(subdir_fifo_fp[ stack_counter ], &tmp_int, sizeof(int));
			dir_info->total_lines_main += tmp_int;
			read(subdir_fifo_fp[ stack_counter ], &tmp_int, sizeof(int));
			if (tmp_int > dir_info->total_threads_main) dir_info->total_threads_main = tmp_int;
			close(subdir_fifo_fp[ stack_counter ]);
			if (unlink(subdir_fifo[ stack_counter ])) {}
			for (tmp_int = 0; subdir_fifo[ stack_counter ][ tmp_int ] != 0; ++tmp_int)
				subdir_fifo[ stack_counter ][ tmp_int ] = 0;
			++stack_counter;
		}
	}
	free(file_info);
	sem_close(sem_logger);
	sem_close(sem_counter);
	closedir(search_dir);
	return dir_info;
}
