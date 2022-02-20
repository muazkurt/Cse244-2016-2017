#ifndef LIST_H
#define LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <types.h>
/**
 * Searches the file for given string.
 * Each time it finds the string,
 * Prints it and searches again until EOF
 **/
return_value *searchFile(const parameters *param,
						 char			   logging_index[ FOUNDABLE_MAX ][ BUFFER_SIZE ]);

/**
 * Works like strcmp, ignores tabs, new lines and spaces.
 **/
int doesStringFound(FILE *openedFile, const char stringtaken[]);

/**
 * Takes a char and makes it lovercase if it is uppercased.
 **/
char makeCharLover(char inputChar);

/**
 * Takes a string and makes it lovercase by using
 * makeCharLover function.
 **/
char *makeStrLover(char *inputChar);
#endif
