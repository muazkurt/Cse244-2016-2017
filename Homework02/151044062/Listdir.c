/*
	Functions
*/
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
	int rowinFile = 1, /*Satýr*/
		columninFile = 1,/*Sütun*/
		foundTimes = 0;
	do {
		/*
		Dosyadan ilk harf okunuyor.
		*/
		oneCharFromFile = makeCharLover(fgetc(openedFile));
		/*
		Eðer yeni satýr karakteri gelirse,
		rowinLine deðiþkeni arttýrýlýp,
		columninFile 1 ile deðiþtiriliyor.
		*/
		if (oneCharFromFile == '\n')
		{
			++rowinFile;
			columninFile = 0;
		}
		/*
		Eðer okunan karakter,
		aranan karakter ile ayný ise;
		Aranan kelimenin bütünü var mý diye
		kontrol etmeye gidiyor.
		*/
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
					fseek(LogFile, 0, SEEK_END);
					searchOpenedFile(openedFileP, searchString, LogFile, address);
				}
				exit(0);
			}
		}
		/*
		Directory ise.
		*/
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