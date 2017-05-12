#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
/*
	Everything about the App.
*/
int ListDirfunction(const char *searchString, char *dirName, FILE *LogFile);

/*
	Does what it's name.
*/
int HowManyFound(FILE *LogFile);

/*
Function from restart.h
*/
int isRegularFile(char *path);


/*
Function from restart.h
*/
int isdirectory(char *path);


/*
r_wait function from restart.h
*/
pid_t r_wait(int *stat_loc);

/*
->Searches the file for given string.
->	Each time founds the string,
->	Prints it and searches again until EOF
*/
int searchOpenedFile(FILE *openedFile, const char stringTaken[], FILE *OutputFile, const char *FileName);
/*
->Works like strcmp, ignores tabs, new lines and spaces.
*/
int doesStringFound(FILE *openedFile, const char stringtaken[]);
/*
Takes a char and makes it lovercase if it is uppercased.
*/
char makeCharLover(char inputChar);
/*
Takes a string and makes it lovercase by using
makeCharLover function.
*/
char *makeStrLover(char *inputChar);
/*
    Takes the string and its input argumants.
    Makes them printable together.
*/
char *makeWritable(char *input, const int row, const int col);
