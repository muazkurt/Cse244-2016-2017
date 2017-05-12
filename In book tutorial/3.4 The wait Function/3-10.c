/*
	The following program creates a process fan. All the forked processes are children of the
original process. How are the output messages ordered?
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
	pid_t childpid = 0;
	int i, n;
	if (argc != 2) { /* check number of command-line arguments */
		fprintf(stderr, "Usage: %s processes\n", argv[0]);
		return 1;
	}
	n = atoi(argv[1]);
	for (i = 1; i < n; i++)
		if ((childpid = fork()) <= 0)
			break;
	for (; ; ) {
		childpid = wait(NULL);
		if ((childpid == -1) && (errno != EINTR))
			break;
	}
	fprintf(stderr, "I am process %ld, my parent is %ld\n",
		(long)getpid(), (long)getppid());
	return 0;
}