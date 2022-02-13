#include "List.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
/**
 * Everything about the App.
**/
int ListDirfunction(const char *searchString, char *dirName, FILE *LogFile);

/**
 * Counts the number of the given file
**/
int HowManyFound(FILE *LogFile);

/**
 * Function from restart.h
**/
int isRegularFile(char *path);

/**
 * Function from restart.h
**/
int isdirectory(char *path);

/**
 * r_wait function from restart.h
**/
pid_t r_wait(int *stat_loc);

/**
 * Takes the string and its input argumants.
 * Makes them printable together.
**/
char *makeWritable(char *input, const int row, const int col);