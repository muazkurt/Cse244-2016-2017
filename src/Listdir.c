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
#define PRINTABLE_MAX 128
#define FOUNDABLE_MAX 8
#define FAILURE_CASE -1

#define LOG_FILENAME "log.log"
typedef struct parmeters_t
{
	const char *base,
		*query;
	char ref[PATH_MAX];
	FILE *logFile;
} parameters;

char *ignoreSpaces(char *inputChar)
{
	int i = 0, max_space = (strlen(inputChar) - strlen("-Fifo"));
	if (inputChar[0] == '.')
		strcpy(inputChar, &inputChar[1]);
	for (i = 0; i < max_space; ++i)
	{
		if (inputChar[i] == ' ')
		{
			inputChar[i] = '\0';
			strcat(inputChar, &inputChar[i + 1]);
		}
	}
	return &inputChar[0];
}

int fileOp(const char *address, const char *dirName, const char *searchString, FILE *logFile)
{
	int _pipe[2] = {0},
		foundTimesParent = 0,
		foundTimesChild = 0;
	pid_t Searcher = 0;
	FILE *openedFileP = NULL;
	if ((strncmp(dirName, LOG_FILENAME, strlen(LOG_FILENAME)) == 0) ||
		(strncmp(dirName, "Listdir", strlen("Listdir")) == 0) ||
		(strncmp(dirName, "Fifo", strlen("Fifo")) == 0))
	{
		fprintf(stderr, "Filepath (%s) is not supported\n", dirName);
		return FAILURE_CASE;
	}

	if (FAILURE_CASE == pipe(_pipe))
	{
		fprintf(stderr, "Failed while creating pipe for file (%s), errno: (%d)\n", dirName, errno);
		return FAILURE_CASE;
	}

	switch (fork())
	{
	case -1:
		fprintf(stderr, "Failed while making for call for file (%s), errno: (%d)\n", dirName, errno);
		r_close(_pipe[0]);
		r_close(_pipe[1]);
		return FAILURE_CASE;
	case 0:
		// Succesfully Created Pipeline
		// fprintf(stderr, "-> %d Child of (%d) working for %s\n", getpid(), getppid(), dirName);
		r_close(_pipe[0]);
		openedFileP = fopen(address, "r");
		fseek(openedFileP, 0, SEEK_SET);
		foundTimesParent = searchOpenedFile(openedFileP, searchString, logFile, address);
		fclose(openedFileP);
		r_write(_pipe[1], &foundTimesParent, sizeof(foundTimesParent));
		r_close(_pipe[1]);
		// fprintf(stderr, "<- %d Child of (%d) quitting %s\n", getpid(), getppid(), dirName);
		exit(0);
	default:
		r_close(_pipe[1]);
		while (r_read(_pipe[0], &foundTimesChild, sizeof(int)) > 0)
			;
		// 	fprintf(stderr, "<- Child for dir %s reported: %d\n", dirName, foundTimesChild);
		// fprintf(stderr, "<------ Child for dir %s reported: %d\n", dirName, foundTimesChild);

		foundTimesParent += foundTimesChild;
		r_close(_pipe[0]);
		break;
	}
	return foundTimesParent;
}

int dirOp(const char *address, const char *dirName, const char *searchString, FILE *logFile)
{
	int fifoInsideOpen = 0,
		Searcher = 0,
		foundTimesChild = 0,
		leave_for_parent = 0;
	char fifoInside[PATH_MAX];

	sprintf(fifoInside, "%s-Fifo", dirName);
	// ignoreSpaces(fifoInside);
	while (FAILURE_CASE == mkfifo(fifoInside, FIFO_PERM))
	{
		if (EEXIST == errno)
		{
			leave_for_parent = 1;
			break;
		}
	}

	fifoInsideOpen = r_open2(fifoInside, O_RDWR);
	switch (Searcher = fork())
	{
	case FAILURE_CASE:
		fprintf(stderr, "Failed while making for call for directory (%s), errno: (%d)\n", dirName, errno);
		return FAILURE_CASE;
	case 0:
		ListDirfunction(searchString, address, logFile, fifoInsideOpen);
		exit(0);

	default:
		while (FAILURE_CASE == (waitpid(Searcher, NULL, 0)) && (errno != EINTR))
			;
		r_read(fifoInsideOpen, &foundTimesChild, sizeof(int));
		break;
	}
	r_close(fifoInsideOpen);
	if (leave_for_parent && FAILURE_CASE == unlink(fifoInside))
	{
		fprintf(stderr, "Failed while unlink the fifo (%s), errno: (%d)\n", fifoInside, errno);
	}
	return foundTimesChild;
}

int ListDirfunction(const char *searchString, const char *dirName, FILE *logFile, int fifoInt)
{
	struct dirent *dirContent = NULL;
	DIR *OpenedDir = NULL;
	pid_t Searcher, IsFinished;
	char address[PATH_MAX],
		fifoInside[PATH_MAX];
	int foundTimesParent = 0,
		fifoInsideOpen = 0;
	if ((OpenedDir = opendir(dirName)) == NULL)
	{
		fprintf(stderr, "Failed to opening directory %s", dirName);
		return FAILURE_CASE;
	}
	while ((dirContent = readdir(OpenedDir)) != NULL)
	{
		sprintf(address, "%s/%s", dirName, dirContent->d_name);
		// File
		if (isRegularFile(address))
		{
			foundTimesParent += fileOp(address, dirContent->d_name, searchString, logFile);
		}
		// Directory
		else if (isdirectory(address))
		{
			foundTimesParent += dirOp(address, dirContent->d_name, searchString, logFile);
		}
		else
			;
	}
	while (FAILURE_CASE == r_wait(NULL) && ECHILD != errno)
	{
		perror("failed: ");
	}
	// while (IsFinished = waitpid(-1, NULL, WNOHANG))
	// 	if ((IsFinished == -1) && (errno != EINTR))
	// 		break;
	r_write(fifoInt, &foundTimesParent, sizeof(int));
	while ((closedir(OpenedDir) == FAILURE_CASE) && (EINTR == errno))
		;
	return 0;
}

int dir(const parameters *param)
{
	int returnVal = EXIT_SUCCESS,
		fifoDescriptor = FAILURE_CASE;
	char fifoFilename[PATH_MAX] = {0};
	snprintf(fifoFilename, PATH_MAX, "%s.fifo", param->base);
	if (FAILURE_CASE == (returnVal = r_mkfifo(fifoFilename, FIFO_PERM)))
		fprintf(stderr, "mkfifo error (%s), errno(%d)\n", fifoFilename, errno);
	else
	{
		if (FAILURE_CASE == (fifoDescriptor = r_open2(fifoFilename, O_RDWR)))
			fprintf(stderr, "open error (%s), errno(%d)\n", fifoFilename, errno);
		else
		{
			ListDirfunction(param->query, param->base, param->logFile, fifoDescriptor);
			int count = FAILURE_CASE;
			if (FAILURE_CASE == r_read(fifoDescriptor, &count, sizeof(int)))
				fprintf(stderr, "read error, errno(%d)\n", errno);
			fseek(param->logFile, 0, SEEK_END);
			fprintf(param->logFile, "\n%d times %s found in %s directory\n", count, param->query, param->base);
		}
		if (fifoDescriptor && FAILURE_CASE == r_close(fifoDescriptor))
			fprintf(stderr, "close error, errno(%d)\n", errno);
	}
	if (returnVal && FAILURE_CASE == (returnVal = unlink("Fifo")))
		fprintf(stderr, "unlink error, errno(%d)\n", errno);
}

int listDir_r(const parameters *param)
{
	int returnVal = EXIT_SUCCESS, fifoCreated = EXIT_FAILURE,
		fifoDescriptor = FAILURE_CASE,
		count = 0;
	char fifoFilename[PATH_MAX] = {0},
		 path[PATH_MAX] = {0};
	snprintf(path, PATH_MAX, "%s%s", param->base, param->ref);
	fprintf(stderr, "path %s\n", path);
	snprintf(fifoFilename, PATH_MAX, "%s.fifo", path);
	fprintf(stderr, "fifoFilename %s\n", fifoFilename);
	DIR *openedDir = NULL;
	struct dirent *dirContent = NULL;
	if (FAILURE_CASE == (returnVal = isdirectory(path)))
		fprintf(stderr, "%s is not a directory\n", path);
	else if (NULL == (openedDir = opendir(path)))
	{
		fprintf(stderr, "Failed while opening directory %s\n", path);
		returnVal = EXIT_FAILURE;
	}
	else if (FAILURE_CASE == (fifoCreated = r_mkfifo(fifoFilename, FIFO_PERM)))
	{
		fprintf(stderr, "mkfifo error (%s), errno(%d)\n", fifoFilename, errno);
		returnVal = EXIT_FAILURE;
	}
	else if (FAILURE_CASE == (fifoDescriptor = r_open2(fifoFilename, O_RDWR)))
	{
		fprintf(stderr, "open error (%s), errno(%d)\n", fifoFilename, errno);
		returnVal = EXIT_FAILURE;
	}
	else
		while (NULL != (dirContent = readdir(openedDir)))
		{
			if (0 == strncmp(dirContent->d_name, LOG_FILENAME, sizeof(LOG_FILENAME) - 1) ||
				0 == strcmp(dirContent->d_name, ".") ||
				0 == strcmp(dirContent->d_name, ".."))
			{
				fprintf(stderr, "Ignoring (%s)\n", dirContent->d_name);
			}
			else
			{
				char address[PATH_MAX] = {0};
				snprintf(address, PATH_MAX, "%s/%s", path, dirContent->d_name);
				switch (dirContent->d_type)
				{
				// Directory
				case DT_DIR:
					switch (fork())
					{
					case FAILURE_CASE:
						fprintf(stderr, "Failed while making for call for directory (%s), errno: (%d)\n", dirContent->d_name, errno);
						break;
					case 0:
					{
						parameters p = {
							.base = param->base,
							.query = param->query,
							.ref = {0},
							.logFile = param->logFile,
						};
						snprintf(p.ref, PATH_MAX, "%s/%s", param->ref, dirContent->d_name);
						fprintf(stderr, "DIR (%s%s)\n", p.base, p.ref);

						listDir_r(&p);
						exit(0);
					}
					default:
						count++;
						break;
					}

					// count += dirOp(param, dirContent->d_name);

					break;
				// File
				case DT_REG:
					fprintf(stderr, "FILE (%s)\n", address);
					count++;
					// foundTimesParent += fileOp(param, dirContent->d_name);
					break;
				default:
					fprintf(stderr, "Unsupported file type. %s\n", address);
					break;
				}
			}
		}

	while (wait(NULL) > 0)
		;

	if (FAILURE_CASE != fifoDescriptor)
	{
		int read_data = -1;
		while (read(fifoDescriptor, &read_data, sizeof(int)))
		{
			fprintf(stderr, "Readdata: %d\n", read_data);
			count += read_data;
		}

		if (FAILURE_CASE == r_close(fifoDescriptor))
			fprintf(stderr, "close error, errno(%d)\n", errno);
	}
	if (FAILURE_CASE != fifoCreated && FAILURE_CASE == (fifoCreated = unlink(fifoFilename)))
	{
		fprintf(stderr, "unlink error, errno(%d)\n", errno);
		returnVal = EXIT_FAILURE;
	}
	if (NULL != openedDir && FAILURE_CASE == (returnVal = closedir(openedDir)))
		fprintf(stderr, "Failed while closing the directory %s", param->base);
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
	listDir_r(&p);
	fclose(logFile);
}