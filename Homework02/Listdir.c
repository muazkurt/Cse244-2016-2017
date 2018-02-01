/*
	Functions
*/
#include "Listdir.h"
void makeCharLover(char * inputChar)
{
	if (*inputChar >= 'A' && *inputChar <= 'Z')
		*inputChar += 'a' - 'A';
	return;
}

const char * const makeStrLover(char * const inputStr)
{
	int charCounter = 0;
	while (*(inputStr + charCounter) != 0)
		makeCharLover(inputStr + charCounter++);
	return inputStr;
}

int searchOn(int openedFile, const char * const stringtaken)
{
	int counter = 0, sameParts = 1, error = 0;
	char char_in;
	while (sameParts != strlen(stringtaken) && error == 0)
	{
		read(openedFile, &char_in, sizeof(char));
		++counter;
		if (char_in == '\n' ||
			char_in == '\t' ||
			char_in == ' ');
		else
		{
			makeCharLover(&char_in);			
			if (char_in == *(stringtaken + sameParts))
				++sameParts;
			else 
				error = 1;
		}
	}
	lseek(openedFile, -(counter * sizeof(char)), SEEK_CUR);
	return (error + 1) % 2;
}


int searchOpenedFile(const file_info * const input, const char * const stringTaken)
{

	char single_char;
	int row = 1, col = 1,
		foundTimes = 0;
	while (read(input->fd, &single_char, sizeof(char)))
	{
		if (single_char == '\n')
		{
			++row;
			col = 0;
		}
		else if(single_char == '\t' || single_char == ' ');
		else 
		{
			makeCharLover(&single_char);
			if (single_char == *stringTaken)
				if (searchOn(input->fd, stringTaken) == 1 && ++foundTimes)
					fprintf(LOGFILE, "%s : [%d, %d] %s first character found.\n", input->filename, row, col, stringTaken);
		}
		++col;
	}
	return foundTimes;
}

int ListDirfunction(const char * const searchString, const char * const dirName)
{
	struct dirent *InputDir = NULL;
	DIR *OpenedDir = NULL;
	pid_t Searcher = 0;
	file_info input;
	if ((OpenedDir = opendir(dirName)) == NULL) return -1;
	while ((InputDir = readdir(OpenedDir)) != NULL)
	{
		if (strcmp(InputDir->d_name, "log.log") == 0 ||
			strcmp(InputDir->d_name, "Listdir") == 0 ||
			strcmp(InputDir->d_name, ".") == 0 ||
			strcmp(InputDir->d_name, "..") == 0);
		else
		{
			sprintf(input.filename, "%s/%s", dirName, InputDir->d_name);
			if (isRegularFile(input.filename))
			{
				if ((Searcher = fork()) == -1)
				{
					perror("Failed to fork.");
					return -1;
				}
				else if (Searcher == 0)
				{
					input.fd = open(input.filename, O_RDONLY);
					lseek(input.fd, 0, SEEK_SET);
					fseek(LOGFILE, 0, SEEK_END);
					searchOpenedFile(&input, searchString);
					exit(0);
				}
			}
			//	FOR DIRECTORY
			else if (isdirectory(input.filename))
			{

				if ((Searcher = fork()) == -1)
				{
					perror("Failed to fork.");
					return -1;
				}
				else if (Searcher == 0)
				{
					ListDirfunction(searchString, input.filename);
					exit(0);
				}
			}
		}
	}
	while (r_wait(NULL) > 0);
	while((closedir(OpenedDir) == -1) && errno == EINTR);
	return 0;
}

int HowManyFound()
{
	int i = 0;
	char untilnewline;
	while ( (untilnewline = fgetc(LOGFILE)) != EOF)
		if (untilnewline == '\n')
			++i;
	return i;
}

pid_t r_wait(int *stat_loc)
{
	pid_t retval;
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