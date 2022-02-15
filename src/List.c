/**
 * Todos: 
 * -> Open the App
 * -> Get the filename
 * -> Get a query as a word or a letter to search in the given file.
 * -> Open the given filename for reading
 * -> For all lines into the given file 
 *  -> Read a line
 *  -> If the query is found in the given line:
 *   -> Print the position of the foundation into a log file as "line number, column"
 *  -> Else:
 *  	-> If there are less letters to the end of the file than the given quuery:
 *    -> Close the file
 *    -> Quit the program 
 * When reach the end of the file, close the file.
 * Clear the used variables.
 * Close the program.
**/
#include "List.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define BUFFER_SIZE 256
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

int writeToFile (FILE *OutputFile, const char * content) {
	int fd = fileno(OutputFile);
	struct flock fl;

	fl.l_type   = F_WRLCK;  /* read/write lock */
	fl.l_whence = SEEK_END; /* beginning of file */
	fl.l_start  = 0;        /* offset from l_whence */
	fl.l_len    = 0;        /* length, 0 = to EOF */
	fl.l_pid    = getpid(); /* PID */
	fcntl(fd, F_SETLKW, &fl); /* set lock */
	fseek(OutputFile, 0, SEEK_END);
	fprintf(OutputFile, "%s", content);
	fl.l_type   = F_UNLCK;
	fcntl(fd, F_SETLK, &fl); /* unset lock */
}

int searchOpenedFile(FILE *openedFile, const char stringTaken[], FILE *OutputFile, const char *FileName)
{
	char buffer[BUFFER_SIZE] = {0};
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
				snprintf(buffer, BUFFER_SIZE, "%s : [%d, %d] %s first character found.\n", FileName, rowinFile, columninFile, stringTaken);
				writeToFile(OutputFile, buffer);
			}
			else
				;
		}
		++columninFile;
	} while (oneCharFromFile != EOF); 
	return foundTimes;
}