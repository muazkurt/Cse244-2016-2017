#include "Listdir.h"
char makeCharLover(char inputChar)
{
	if (inputChar >= 'A' && inputChar <= 'Z')
		inputChar += 'a' - 'A';
	else;
	return inputChar;
}

char *makeStrLover(char *inputStr)
{
	int charCounter = 0;
	while (charCounter < strlen(inputStr))
	{
		inputStr[charCounter] = makeCharLover(inputStr[charCounter]);
		++charCounter;
	}
	return inputStr;
}

int doesStringFound(FILE *openedFile, const char stringtaken[])
{
	int readCharactersFromFile = 0, sameParts = 1, error = 0;
	char charfromFile = 0;
	fpos_t pos;
	fgetpos(openedFile, &pos);
	while (sameParts != strlen(stringtaken) && error != 1)
	{
		charfromFile = fgetc(openedFile);
		charfromFile = makeCharLover(charfromFile);
		++readCharactersFromFile;
		if (charfromFile == '\n' || charfromFile == '\t' || charfromFile == ' ');
		else if (charfromFile == stringtaken[sameParts])
			++sameParts;
		else if (charfromFile == EOF || charfromFile != stringtaken[sameParts])
			error = 1;
	}
	fsetpos(openedFile, &pos);
	if (error == 0)
		return 1;
	return 0;
}

int searchOpenedFile(FILE *openedFile, const char stringTaken[], FILE *OutputFile, const char *FileName)
{
	char oneCharFromFile = 0;
	int rowinFile = 1, /*Line*/
		columninFile = 1,/*Column*/
		foundTimes = 0;
	do{
		// Reads the first character from the file.
		oneCharFromFile = makeCharLover(fgetc(openedFile));
		// If the character is '\n', 
		// Increase the rowinLine and set columninLine to 1.
		if (oneCharFromFile == '\n')
		{
			++rowinFile;
			columninFile = 0;
		}
		// If the character is the same with the starting char of the given query,
		// Start to search for all the query.
		else if (oneCharFromFile == stringTaken[0])
		{
			if (doesStringFound(openedFile, stringTaken) == 1)
			{
				++foundTimes;
				fprintf(OutputFile, "%s : [%d, %d] %s first character found.\n", FileName, rowinFile, columninFile, stringTaken);
			}
			else
				;
		}
		++columninFile;
	} while (oneCharFromFile != EOF); 
	return foundTimes;
}

char *ignoreSpaces(char *inputChar)
{
	int i = 0, max_space = (strlen(inputChar) - strlen("-Fifo"));
	if(inputChar[0] == '.')
		strcpy(inputChar, &inputChar[1]);
	for(i = 0; i < max_space; ++i)
	{
		if(inputChar[i] == ' ')
		{
			inputChar[i] = '\0';
			strcat(inputChar, &inputChar[i + 1]);
		}
	}
	return &inputChar[0];
}

int ListDirfunction(const char *searchString, const char *dirName, FILE *LogFile, int fifoInt)
{
	struct dirent *InputDir = NULL;
	DIR *OpenedDir = NULL;
	pid_t Searcher, IsFinished;
	FILE *openedFileP = NULL;
	char address[PATH_MAX],
		foundTimesChild[FOUNDABLE_MAX],
		fifoInside[PATH_MAX];
	int fd[2],
		foundTimesParent = 0,
		fifoInsideOpen = 0;
	if ((OpenedDir = opendir(dirName)) == NULL)
	{
		fprintf(stderr, "Failed to opening directory %s", dirName);
		return -1;
	}
	while ((InputDir = readdir(OpenedDir)) != NULL)
	{
		sprintf(address, "%s/%s", dirName, InputDir->d_name);
		// File
		if (isRegularFile(address))
		{
			foundTimesChild[0] = 0;
			if ((strncmp(InputDir->d_name, "log.log", strlen("log.log")) == 0) ||
				(strncmp(InputDir->d_name, "Listdir", strlen("Listdir")) == 0) ||
				(strncmp(InputDir->d_name, "Fifo", strlen("Fifo")) == 0));
			else
			{
				if (pipe(fd) == -1) perror("Failed to creating pipe!");
				else if((Searcher = fork()) == -1) perror("Failed to fork for File!");
				//Succesfully Created Pipeline
				if (Searcher == 0)
				{
					openedFileP = fopen(address, "r");
					fseek(openedFileP, 0, SEEK_SET);
					fseek(LogFile, 0, SEEK_END);
					foundTimesParent = searchOpenedFile(openedFileP, searchString, LogFile, address);
					sprintf(foundTimesChild, "%d", foundTimesParent);
					r_write(fd[1], foundTimesChild, (strlen(foundTimesChild) + 1));
					while ((close(fd[1]) == -1) && (errno == EINTR));
					fclose(openedFileP);
					exit(0);
				}
				r_read(fd[0], foundTimesChild, FOUNDABLE_MAX);
				foundTimesParent += atoi(foundTimesChild);
				while ((close(fd[0]) == -1) && (errno == EINTR));
			}
		}
		// Directory
		else if (isdirectory(address))
		{
			if ((strcmp(InputDir->d_name, ".") == 0) || (strcmp(InputDir->d_name, "..") == 0))
				;
			else
			{
				sprintf(fifoInside, "%s-Fifo", InputDir->d_name);
				ignoreSpaces(fifoInside);
				while (mkfifo(fifoInside, FIFO_PERM) == -1)
				{
					if(errno == EINTR)
						;
					else if(errno == EEXIST)
						break;
				}
				while(((fifoInsideOpen = open(fifoInside, O_RDWR)) == -1) && (errno == EINTR));
				if ((Searcher = fork()) == -1)
					perror("Failed to fork for Directory!");
				if (Searcher >= 0)
				{
					if(Searcher == 0)
					{
						ListDirfunction(searchString, address, LogFile, fifoInsideOpen);
						exit(0);
					}
					while((waitpid(Searcher, NULL, WNOHANG) != -1) && (errno != EINTR));
					r_read(fifoInsideOpen, foundTimesChild, FOUNDABLE_MAX);
					foundTimesParent += atoi(foundTimesChild);
				}
				while((close(fifoInsideOpen) == -1) && (errno == EINTR));
				if(unlink(fifoInside) == -1) 
					perror("Error to unlink fifoInside");
			}
		}
		else
			;
	}
	//while(r_wait(NULL) > 0);
	while (IsFinished = waitpid(-1, NULL, WNOHANG))
		if ((IsFinished == -1) && (errno != EINTR))
			break;
	sprintf(foundTimesChild, "%d", foundTimesParent);
	r_write(fifoInt, foundTimesChild, (strlen(foundTimesChild) + 1));
	while((closedir(OpenedDir) == -1) && (errno == EINTR));
	return 0;
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

ssize_t r_write(int fd, void *buf, size_t size) {
	char *bufp;
	size_t bytestowrite;
	ssize_t byteswritten;
	size_t totalbytes;
	for (bufp = buf, bytestowrite = size, totalbytes = 0;
		bytestowrite > 0;
		bufp += byteswritten, bytestowrite -= byteswritten) {
		byteswritten = write(fd, bufp, bytestowrite);
		if ((byteswritten) == -1 && (errno != EINTR))
			return -1;
		if (byteswritten == -1)
			byteswritten = 0;
		totalbytes += byteswritten;
	}
	return totalbytes;
}

ssize_t r_read(int fd, void *buf, size_t size) {
	ssize_t retval;
	while (retval = read(fd, buf, size), retval == -1 && errno == EINTR);
	return retval;
}