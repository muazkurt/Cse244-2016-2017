/*
	The following program outputs its process ID and its parent process ID. Notice that the return
values are cast to long for printing since there is no guarantee that a pid_t will fit in an int.
*/
#include <stdio.h>
#include <unistd.h>
int main(void) {
	printf("I am process %ld\n", (long)getpid());
	printf("My parent is %ld\n", (long)getppid());
	return 0;
}