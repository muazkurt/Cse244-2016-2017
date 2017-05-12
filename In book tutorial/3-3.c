/*
	In the following program, both parent and child execute the x = 1 assignment statement after
returning from fork.
*/
#include <stdio.h>
#include <unistd.h>
int main(void) {
	int x;
	x = 0;
	fork();
	x = 1;
	printf("I am process %ld and my x is %d\n", (long)getpid(), x);
	return 0;
}