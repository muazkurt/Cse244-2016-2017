/**
 * Todos:
 * -> Open the App
 * -> Get the filename
 * -> Get a query as a word or a letter to search in the given file.
 * -> Open the given filename for reading
 * -> For all lines into the given file
 *  -> Read a line
 *  -> If the query is found in the given line:
 *   -> Print the position of the foundation into a log file as "line number,
 *		column"
 *  -> Else:
 *  	-> If there are less letters to the end of the file than the given
 *			quuery:
 *    -> Close the file
 *    -> Quit the program
 * When reach the end of the file, close the file.
 * Clear the used variables.
 * Close the program.
 **/
#include "List.h"

#include <fcntl.h>
#include <pthread.h>
#include <restart.h>
#include <string.h>
#include <unistd.h>


char makeCharLover(char inputChar) {
	if (inputChar >= 'A' && inputChar <= 'Z') inputChar += 'a' - 'A';
	return inputChar;
}

char *makeStrLover(char *inputStr) {
	int charCounter = 0;
	while (charCounter < strlen(inputStr)) {
		inputStr[ charCounter ] = makeCharLover(inputStr[ charCounter ]);
		++charCounter;
	}
	return inputStr;
}

int doesStringFound(FILE *openedFile, const char stringtaken[]) {
	int	   readCharactersFromFile = 0, sameParts = 1, error = 0;
	char   charfromFile = 0;
	fpos_t pos;
	fgetpos(openedFile, &pos);
	while (sameParts != strlen(stringtaken) && error != 1) {
		charfromFile = fgetc(openedFile);
		charfromFile = makeCharLover(charfromFile);
		++readCharactersFromFile;
		if (charfromFile == '\n' || charfromFile == '\t' || charfromFile == ' ') {
		} else if (charfromFile == stringtaken[ sameParts ])
			++sameParts;
		else if (charfromFile == EOF || charfromFile != stringtaken[ sameParts ])
			error = 1;
	}
	fsetpos(openedFile, &pos);
	if (error == 0) return 1;
	return 0;
}

int log_all(char **index, const parameters *param) {
	while (sem_wait(param->log_sem) == -1) continue;
	int i = 0;
	while (pthread_rwlock_wrlock(&param->logging) != 0) continue;
	for (i = 0; i < FOUNDABLE_MAX && 0 != index[ i ]; ++i)
		r_write(param->logFile, &index[ i ], BUFFER_SIZE * sizeof(char));
	while (pthread_rwlock_unlock(&param->logging) != 0) continue;
	while (sem_post(param->log_sem) == -1) continue;
	return 0;
}

return_value searchFile(const parameters *param) {
	char path[ PATH_MAX ] = {0}, oneCharFromFile = 0,
						logging_index[ FOUNDABLE_MAX ][ BUFFER_SIZE ] = {0};
	int line_count													  = 1, /*Line*/
		columninFile												  = 1, /*Column*/
		file_dest													  = FAILURE_CASE;
	return_value file_info											  = {0};

	if (NULL == param) {
		fprintf(stderr, "Param is NULL (pid: %d)", getpid());
		return file_info;
	}
	increaseWorkingCount(param->counter_sem);

	snprintf(path, PATH_MAX, "%s%s", param->base, param->ref);
	if (FAILURE_CASE == (file_dest = r_open2(path, O_RDONLY))) {
		fprintf(stderr, "Open %s is failed (pid: %d)", path, getpid());
		return file_info;
	}
	// Reads the first character from the file.
	while (r_read(file_dest, &oneCharFromFile, sizeof(char)) > 0) {
		oneCharFromFile = makeCharLover(oneCharFromFile);
		if (oneCharFromFile == '\n') {
			// If the character is '\n',
			// Increase the rowinLine and set columninLine to 1.
			++file_info.line_count;
			columninFile = 0;
		} else if (oneCharFromFile == param->query[ 0 ]) {
			// If the character is the same with the starting char of the given
			// query, Start to search for all the query.
			if (doesStringFound(file_dest, param->query) == 1) {
				++file_info.occurences;
				snprintf(logging_index[ file_info.occurences - 1 ],
						 BUFFER_SIZE,
						 "%ld %ld %s : [%d, %d] %s first character is found.\n",
						 (long) getpid(),
						 (long) pthread_self(),
						 path,
						 file_info.line_count,
						 columninFile,
						 param->query);
			}
		}
		++columninFile;
	}
	if (log_all(logging_index, param) != 0)
		fprintf(stderr,
				"Failed to log all. (pid: %d, file %s/%s)",
				getpid(),
				param->base,
				param->ref);
	decreaseWorkingCount(param->counter_sem);
	return file_info;
}