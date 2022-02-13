#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Searches the file for given string.
 * Each time it finds the string,
 * Prints it and searches again until EOF
**/
int searchOpenedFile(FILE *openedFile, const char stringTaken[], FILE *OutputFile, const char *FileName);

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