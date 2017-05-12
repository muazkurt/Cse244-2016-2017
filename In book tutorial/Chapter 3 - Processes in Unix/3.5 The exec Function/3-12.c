/*
	A program that creates a child process to run ls -l.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
	pid_t childpid;
	childpid = fork();
	if (childpid == -1) {
		perror("Failed to fork");
		return 1;
	}
	if (childpid == 0) { /* child code */
		execl("/bin/ls", "ls", "-al", NULL);
		perror("Child failed to exec ls");
		return 1;
	}
	if (childpid != wait(NULL)) { /* parent code */
		perror("Parent failed to wait due to signal or error");
		return 1;
	}
	return 0;
}