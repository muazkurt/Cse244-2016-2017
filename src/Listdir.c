#include "Listdir.h"
int ListDirfunction(const char *searchString, char *dirName, FILE *LogFile)
{
	struct dirent *InputDir = NULL;
	DIR *OpenedDir = NULL;
	pid_t Searcher = NULL;
	FILE *openedFileP = NULL;
	char address[PATH_MAX];
	if ((OpenedDir = opendir(dirName)) == NULL) return -1;
	while ((InputDir = readdir(OpenedDir)) != NULL)
	{
		sprintf(address, "%s/%s", dirName, InputDir->d_name);
		if (isRegularFile(address))
		{
			if ((Searcher = fork()) == -1)
			{
				perror("Failed to fork.");
				return -1;
			}
			else if (Searcher == 0)
			{
				if (strcmp(InputDir->d_name, "log.log") == 0 || strcmp(InputDir->d_name, "Listdir") == 0);
				else
				{
					openedFileP = fopen(address, "r");
					fseek(openedFileP, 0, SEEK_SET);
					searchOpenedFile(openedFileP, searchString, LogFile, address);
				}
				exit(0);
			}
		}
		// Directory
		else if (isdirectory(address))
		{
			if (strcmp(InputDir->d_name, ".") == 0 || strcmp(InputDir->d_name, "..") == 0);
			else
			{
				if ((Searcher = fork()) == -1)
				{
					perror("Failed to fork.");
					return -1;
				}
				else if (Searcher == 0)
				{
					ListDirfunction(searchString, address, LogFile);
					exit(0);
				}
			}
		}
	}
	while (r_wait(NULL) > 0);
	while((closedir(OpenedDir) == -1) && errno == EINTR);
	return 0;
}

int HowManyFound(FILE *LogFile)
{
	int i = 0;
	char untilnewline = NULL;
	while ( (untilnewline = fgetc(LogFile)) != EOF)
		if (untilnewline == '\n')
			++i;
	return i;
}

pid_t r_wait(int *stat_loc)
{
	pid_t retval = NULL;
	while (((retval = wait(stat_loc)) == -1) && (errno == EINTR));
	return retval;
}

int isRegularFile(char *path)
{
	struct stat statbuf;
	if (stat(path, &statbuf) == -1)
		return 0;
	else
		return S_ISREG(statbuf.st_mode);
}

int isdirectory(char *path)
{
	struct stat statbuf;
	if (stat(path, &statbuf) == -1)
		return 0;
	else
		return S_ISDIR(statbuf.st_mode);
}