#ifndef LISTDIR_H
#define LISTDIR_H
#include "List.h"
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
/**
 * Counts the number of the given file
**/
int HowManyFound(FILE *LogFile);

/**
 * Takes the string and its input argumants.
 * Makes them printable together.
**/
char *makeWritable(char *input, const int row, const int col);

/**
 * Everything about the App.
**/
int ListDirfunction(const char *searchString, const char *dirName, FILE *LogFile, int fifoInt);


/**
 * Caller for the App.
**/
int listDir(const char *query, const char *path);
#endif
