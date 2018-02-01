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

int ListDirfunction(const char * const searchString, const char * const dirName,  int fifoInt)
{
	struct dirent *InputDir = NULL;
	DIR *OpenedDir = NULL;
	pid_t Searcher = 0;
	file_info input, fifodest;

	int fd[2],
		child_found = 0,
		temp = 0;

	if ((OpenedDir = opendir(dirName)) == NULL)
	{
		fprintf(stderr, "Failed to opening directory %s", dirName);
		return -1;
	}
	while ((InputDir = readdir(OpenedDir)) != NULL)
	{
		if ((strncmp(InputDir->d_name, "log.log", strlen("log.log")) == 0) ||
			(strncmp(InputDir->d_name, "Listdir", strlen("Listdir")) == 0) ||
			(strncmp(InputDir->d_name, "Fifo", strlen("Fifo")) == 0) ||
			(strcmp(InputDir->d_name, ".") == 0) ||
			(strcmp(InputDir->d_name, "..") == 0));
		else
		{
			sprintf(input.filename, "%s/%s", dirName, InputDir->d_name);		
			if (isRegularFile(input.filename))
			{
				if (pipe(fd) == -1) perror("Failed to creating pipe!");
				else if((Searcher = fork()) == -1) perror("Failed to fork for File!");
				/*
					Succesfully Created Pipeline
				*/
				if (Searcher == 0)
				{
					input.fd = open(input.filename, O_RDONLY);
					lseek(input.fd, 0, SEEK_SET);
					fseek(LOGFILE, 0, SEEK_END);
					child_found = searchOpenedFile(&input, searchString);
					r_write(fd[1], &child_found, sizeof(int));
					while ((close(fd[1]) == -1) && (errno == EINTR));
					close(input.fd);
					exit(0);
				}
				r_read(fd[0], &temp, sizeof(int));
				child_found += temp;
				while ((close(fd[0]) == -1) && (errno == EINTR));
			}
			/*
			Directory ise.
			*/
			else if (isdirectory(input.filename))
			{
				sprintf(fifodest.filename, "%s-Fifo", InputDir->d_name);
				ignoreSpaces(fifodest.filename);
				while (mkfifo(fifodest.filename, FIFO_PERM) == -1)
				{
					if(errno == EINTR);
					else if(errno == EEXIST)
						break;
				}
				while(((fifodest.fd = open(fifodest.filename, O_RDWR)) == -1) && (errno == EINTR));
				if ((Searcher = fork()) == -1)
					perror("Failed to fork for Directory!");
				if (Searcher >= 0)
				{
					if(Searcher == 0)
					{
						ListDirfunction(searchString, input.filename, fifodest.fd);
						exit(0);
					}
					while((waitpid(Searcher, NULL, WNOHANG) != -1) && (errno != EINTR));
					r_read(fifodest.fd, &temp, sizeof(int));
					child_found += temp;
				}
				while((close(fifodest.fd) == -1) && (errno == EINTR));
				if(unlink(fifodest.filename) == -1) 
					perror(fifodest.filename);
			}
		}
	}
	while(r_wait(NULL) > 0);
	r_write(fifoInt, &child_found, sizeof(int));
	while((closedir(OpenedDir) == -1) && (errno == EINTR));
	return 0;
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

