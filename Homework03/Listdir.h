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

#define PATH_MAX 255
FILE * LOGFILE;


typedef struct
{
    int fd;
    char filename[PATH_MAX];
} file_info;





/*
	Everything about the App.
*/
int ListDirfunction(const char * const searchString, const char * const dirName, int fifoInt);

/*
Function from restart.h
*/
int isRegularFile(char *path);

/*
Function from restart.h
*/
int isdirectory(char *path);

/*
	restart.h Function.
*/
ssize_t r_write(int fd, void *buf, size_t size);

/*
	restart.h Function.
*/
ssize_t r_read(int fd, void *buf, size_t size);

/*
r_wait function from restart.h
*/
pid_t r_wait(int *stat_loc);

/*
->Searches the file for given string.
->	Each time founds the string,
->	Prints it and searches again until EOF
*/
int searchOpenedFile(const file_info * const, const char * const);


/*
    ->Works like strcmp, ignores tabs, new lines and spaces.
*/
int searchOn(int, const char * const);


/*
    Takes a char and makes it lovercase if it is uppercased.
*/
void makeCharLover(char *);


/*
    Takes a string and makes it lovercase by using
    makeCharLover function.
*/
const char * const makeStrLover(char * const);
