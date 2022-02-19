#include "Listdir.h"
#include "restart.h"
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#ifndef __USE_MISC
#define __USE_MISC
#endif
#ifndef PATH_MAX
#define PATH_MAX 256
#endif
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#define MAX_FILE_COUNT 16
#define FAILURE_CASE -1

#define PARENT_READ 0
#define PARENT_WRITE 1
#define CHILD_READ 0
#define CHILD_WRITE 1
#define FALSE 0
#define TRUE 1

#define LOG_FILENAME "log.log"
/**
 * @param base is the given path,
 * @param query is the requested keyword or letter to search for.
 * @param ref is the currently working subdir of base path
 * @param logFile is the output file for logging.
 **/
typedef struct parmeters_t
{
	const char *base,
		*query;
	char ref[PATH_MAX];
	FILE *logFile;
} parameters;

int fileOp(const parameters *param)
{
	char path[PATH_MAX] = {0};
	int found_times = 0;
	FILE *openedFileP = NULL;
	snprintf(path, PATH_MAX, "%s%s", param->base, param->ref);

	openedFileP = fopen(path, "r");
	fseek(openedFileP, 0, SEEK_SET);
	found_times = searchOpenedFile(openedFileP, param->query, param->logFile, path);
	fclose(openedFileP);
	return found_times;
}

int collectPipes(int pipes[MAX_FILE_COUNT][2], int pipeCount)
{
	int collects = 0;
	while (0 < pipeCount)
	{
		--pipeCount;
		int read_data = -1;
		while (r_read(pipes[pipeCount][PARENT_READ], &read_data, sizeof(int)) > 0)
			collects += read_data;
		r_close(pipes[pipeCount][PARENT_READ]);
	}
	return collects;
}
int collectFifo(const char *fifoPath)
{
	int collects = 0, fifoDescriptor = -1;
	if (FAILURE_CASE == (fifoDescriptor = r_open2(fifoPath, O_RDONLY)))
		fprintf(stderr, "open error (%s), errno(%d)\n", fifoPath, errno);
	else
	{
		int read_data = -1;
		while (wait(NULL) > 0)
			;
		while (r_read(fifoDescriptor, &read_data, sizeof(int)) > 0)
			collects += read_data;
		if (FAILURE_CASE == r_close(fifoDescriptor))
			fprintf(stderr, "close error, errno(%d)\n", errno);
	}
	return collects;
}

int listDir_r(const parameters *param)
{
	int returnVal = FAILURE_CASE, fifoCreated = FAILURE_CASE, has_subdir = FALSE,
		fifoDescriptor = FAILURE_CASE, pipe_descrpitor[MAX_FILE_COUNT][2] = {0},
		pipe_created = 0,
		find_count = 0;
	char fifoFilename[PATH_MAX] = {0},
		 path[PATH_MAX] = {0};
	snprintf(path, PATH_MAX, "%s%s", param->base, param->ref);
	snprintf(fifoFilename, PATH_MAX, "%s.fifo", path);
	DIR *openedDir = NULL;
	struct dirent *dirContent = NULL;
	if (FAILURE_CASE == (returnVal = isdirectory(path)))
	{
		fprintf(stderr, "%s is not a directory\n", path);
		returnVal = FAILURE_CASE;
	}
	else if (NULL == (openedDir = opendir(path)))
	{
		fprintf(stderr, "Failed while opening directory %s\n", path);
		returnVal = FAILURE_CASE;
	}
	else if (FAILURE_CASE == (fifoCreated = r_mkfifo(fifoFilename, FIFO_PERM)))
	{
		fprintf(stderr, "mkfifo error (%s), errno(%d)\n", fifoFilename, errno);
		returnVal = FAILURE_CASE;
	}
	else
		while (pipe_created < MAX_FILE_COUNT && NULL != (dirContent = readdir(openedDir)))
		{
			if (0 == strncmp(dirContent->d_name, LOG_FILENAME, sizeof(LOG_FILENAME) - 1) ||
				0 == strcmp(dirContent->d_name, ".") ||
				0 == strcmp(dirContent->d_name, ".."))
				--pipe_created;
			else
			{
				parameters p = {
					.base = param->base,
					.query = param->query,
					.ref = {0},
					.logFile = param->logFile,
				};
				snprintf(p.ref, PATH_MAX, "%s/%s", param->ref, dirContent->d_name);
				int childProcess = FAILURE_CASE;
				if (FAILURE_CASE == pipe(pipe_descrpitor[pipe_created]))
					fprintf(stderr, "Failed to create a pipe for (%s), errno: (%d)\n", dirContent->d_name, errno);
				else if (FAILURE_CASE == (childProcess = fork()))
					fprintf(stderr, "Failed while fork for(%s), errno: (%d)\n", dirContent->d_name, errno);
				else
					switch (dirContent->d_type)
					{
					case DT_DIR: // Directory
						r_close(pipe_descrpitor[pipe_created][CHILD_READ]);
						r_close(pipe_descrpitor[pipe_created][CHILD_WRITE]);
						--pipe_created;
						has_subdir = TRUE;
						if (0 == childProcess)
						{
							if (FAILURE_CASE == (find_count = listDir_r(&p)))
							{
								fprintf(stderr, "Error: pid(%d), path(%s/%s).\n", getpid(), path, dirContent->d_name);
								exit(FAILURE_CASE);
							}
							else if (FAILURE_CASE == (fifoDescriptor = r_open2(fifoFilename, O_WRONLY)))
							{
								fprintf(stderr, "open error (%s), errno(%d)\n", fifoFilename, errno);
								returnVal = FAILURE_CASE;
							}
							else
							{
								r_write(fifoDescriptor, &find_count, sizeof(int));
								if (FAILURE_CASE == r_close(fifoDescriptor))
									fprintf(stderr, "close error, errno(%d)\n", errno);
							}
							exit(EXIT_SUCCESS);
						}
						break;
					case DT_REG: // File
						if (0 == childProcess)
						{
							r_close(pipe_descrpitor[pipe_created][CHILD_READ]);
							if (FAILURE_CASE == (find_count = fileOp(&p)))
							{
								fprintf(stderr, "Error: pid(%d), path(%s/%s).\n", getpid(), path, dirContent->d_name);
								exit(FAILURE_CASE);
							}
							else
							{
								r_write(pipe_descrpitor[pipe_created][CHILD_WRITE], &find_count, sizeof(int));
								r_close(pipe_descrpitor[pipe_created][CHILD_WRITE]);
							}
							exit(EXIT_SUCCESS);
						}
						else
							r_close(pipe_descrpitor[pipe_created][PARENT_WRITE]);
						break;
					default:
						if (0 == childProcess)
						{
							r_close(pipe_descrpitor[pipe_created][CHILD_READ]);
							r_close(pipe_descrpitor[pipe_created][CHILD_WRITE]);
							exit(FAILURE_CASE);
						}
						fprintf(stderr, "Unsupported file type. %s\n", param->ref);
						break;
					}
			}
			++pipe_created;
		}
	find_count = collectPipes(pipe_descrpitor, pipe_created);
	if (has_subdir && FAILURE_CASE != fifoCreated)
		find_count += collectFifo(fifoFilename);
	if (FAILURE_CASE != fifoCreated && FAILURE_CASE == (fifoCreated = unlink(fifoFilename)))
	{
		fprintf(stderr, "unlink error, errno(%d)\n", errno);
		returnVal = FAILURE_CASE;
	}

	if (NULL != openedDir && FAILURE_CASE == (fifoCreated = closedir(openedDir)))
		fprintf(stderr, "Failed while closing the directory %s", param->base);

	if (FAILURE_CASE != returnVal)
	{
		returnVal = find_count;
	}
	return returnVal;
}

int listDir(const char *query, const char *path)
{
	FILE *logFile = fopen(LOG_FILENAME, "w+");
	parameters p = {
		.base = path,
		.query = query,
		.ref = {0},
		.logFile = logFile,
	};
	fprintf(logFile, "%d '%s' were found in total.\n", listDir_r(&p), query);
	fclose(logFile);
}