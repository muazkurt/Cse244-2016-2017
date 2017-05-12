#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "restart.h"
#define FIFO_MODES O_RDONLY
#define BUFSIZE 256
#define FIFO_PERM (S_IRUSR | S_IWUSR)
int dofifochild(const char *fifoname, const char *idstring);
int dofifoparent(const char *fifoname);
int main(int argc, char *argv[]) {
	pid_t childpid;
	if (argc != 2) { /* command line has pipe name */
		fprintf(stderr, "Usage: %s pipename\n", argv[0]);
		return 1;
	}
	if (mkfifo(argv[1], FIFO_PERM) == -1) { /* create a named pipe */
		if (errno != EEXIST) {
			fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
				(long)getpid(), argv[1], strerror(errno));
			return 1;
		}
	}
	if ((childpid = fork()) == -1) {
		perror("Failed to fork");
		return 1;
	}
	if (childpid == 0) /* The child writes */
		return dofifochild(argv[1], "this was written by the child");
	else
		return dofifoparent(argv[1]);
}
int dofifoparent(const char *fifoname) {
	char buf[BUFSIZE];
	int fd;
	int rval;
	fprintf(stderr, "[%ld]:(parent) about to open FIFO %s...\n",
		(long)getpid(), fifoname);
	while (((fd = open(fifoname, FIFO_MODES)) == -1) && (errno == EINTR));
	if (fd == -1) {
		fprintf(stderr, "[%ld]:failed to open named pipe %s for read: %s\n",
			(long)getpid(), fifoname, strerror(errno));
		return 1;
	}
	fprintf(stderr, "[%ld]:about to read...\n", (long)getpid());
	rval = r_read(fd, buf, BUFSIZE);
	if (rval == -1) {
		fprintf(stderr, "[%ld]:failed to read from pipe: %s\n",
			(long)getpid(), strerror(errno));
		return 1;
	}
	fprintf(stderr, "[%ld]:read %.*s\n", (long)getpid(), rval, buf);
	return 0;
}
int dofifochild(const char *fifoname, const char *idstring) {
	char buf[BUFSIZE];
	int fd;
	int rval;
	ssize_t strsize;
	fprintf(stderr, "[%ld]:(child) about to open FIFO %s...\n",
		(long)getpid(), fifoname);
	while (((fd = open(fifoname, O_WRONLY)) == -1) && (errno == EINTR));
	if (fd == -1) {
		fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
			(long)getpid(), fifoname, strerror(errno));
		return 1;
	}
	rval = snprintf(buf, BUFSIZE, "[%ld]:%s\n", (long)getpid(), idstring);
	if (rval < 0) {
		fprintf(stderr, "[%ld]:failed to make the string:\n", (long)getpid());
		return 1;
	}
	strsize = strlen(buf) + 1;
	fprintf(stderr, "[%ld]:about to write...\n", (long)getpid());
	rval = r_write(fd, buf, strsize);
	if (rval != strsize) {
		fprintf(stderr, "[%ld]:failed to write to pipe: %s\n",
			(long)getpid(), strerror(errno));
		return 1;
	}
	fprintf(stderr, "[%ld]:finishing...\n", (long)getpid());
	return 0;
}