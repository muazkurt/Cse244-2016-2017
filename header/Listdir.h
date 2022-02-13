#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define PRINTABLE_MAX 128
#define FOUNDABLE_MAX 8
/**
 * Everything about the App.
**/
int ListDirfunction(const char *searchString, const char *dirName, FILE *LogFile, int fifoInt);


/**
 * Function from restart.h
**/
int isRegularFile(char *path);


/**
 * Function from restart.h
**/
int isdirectory(char *path);

/**
 * Function from restart.h
**/
ssize_t r_write(int fd, void *buf, size_t size);

/**
 * Function from restart.h
**/
ssize_t r_read(int fd, void *buf, size_t size);

/**
 * r_wait function from restart.h
**/
pid_t r_wait(int *stat_loc);

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
