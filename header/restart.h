#ifndef RESTART_H
#define RESTART_H
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

/**
 * r_wait function from restart.h
**/
pid_t r_wait(int *stat_loc);

/**
 * Function from restart.h
**/
int isRegularFile(const char *path);

/**
 * Function from restart.h
**/
int isdirectory(const char *path);

/**
 * Function from restart.h
**/
ssize_t r_write(int fd, void *buf, size_t size);

/**
 * Function from restart.h
**/
ssize_t r_read(int fd, void *buf, size_t size);

/**
 * Function from restart.h
**/
int r_close(int fildes);

/**
 * Function from restart.h
**/
int r_open2(const char *path, int oflag);


/**
 * Function to wrap mkfifo in a loop.
**/ 
int r_mkfifo(const char *path, int oflag);

#endif