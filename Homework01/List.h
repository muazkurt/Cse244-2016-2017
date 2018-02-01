#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
    Takes a char and makes it lovercase if it is uppercased.
*/
void makeCharLover(char *);


/*
    Takes a string and makes it lovercase by using
    makeCharLover function.
*/
const char * const makeStrLover(char * const);


/*
    ->Searches the file for given string.
    ->	Each time founds the string,
    ->	Prints it and searches again until EOF
*/
int searchfile(int, const char * const);


/*
    ->Works like strcmp, ignores tabs, new lines and spaces.
*/
int searchOn(int, const char * const);