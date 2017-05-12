/*
	The following modification of the process fan of Program 3.2 causes the original process to print
out its information after all children have exited.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/wait.h>
pid_t r_wait(int *stat_loc) {
	int retval;
	while (((retval = wait(stat_loc)) == -1) && (errno == EINTR));
	return retval;
}
int main(int argc, char *argv[]) {
	pid_t childpid;
	int i, n;
	if (argc != 2) {
		fprintf(stderr, "Usage: %s n\n", argv[0]);
		return 1;
	}
	n = atoi(argv[1]);
	for (i = 1; i < n; i++)
		if ((childpid = fork()) <= 0)
			break;
	while (r_wait(NULL) > 0); /* wait for all of your children */
	fprintf(stderr, "i:%d process ID:%ld parent ID:%ld child ID:%ld\n",
		i, (long)getpid(), (long)getppid(), (long)childpid);
	return 0;
}