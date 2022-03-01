#ifndef LIST_H
#define LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

unsigned getMaxWorkers();

/**
 * Searches the file for given string.
 * Each time it finds the string,
 * Prints it and searches again until EOF
 **/
parameters *searchFile(parameters *param);

/**
 * Works like strcmp, ignores tabs, new lines and spaces.
 **/
int doesStringFound(int openedFile, const char stringtaken[]);

#endif
