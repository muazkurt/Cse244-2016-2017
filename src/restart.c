#include "restart.h"
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t r_wait(int *stat_loc)
{
	pid_t retval = 0;
	while (((retval = wait(stat_loc)) == -1) && (errno == EINTR))
		;
	return retval;
}

int isRegularFile(const char *path)
{
	struct stat statbuf = {0};
	if (stat(path, &statbuf) == -1)
		return 0;
	else
		return S_ISREG(statbuf.st_mode);
}

int isdirectory(const char *path)
{
	struct stat statbuf = {0};
	if (stat(path, &statbuf) == -1)
		return 0;
	else
		return S_ISDIR(statbuf.st_mode);
}

ssize_t r_write(int fd, void *buf, size_t size)
{
	char *bufp = NULL;
	size_t bytestowrite = 0,
		   byteswritten = 0,
		   totalbytes = 0;
	for (bufp = buf, bytestowrite = size, totalbytes = 0;
		 bytestowrite > 0;
		 bufp += byteswritten, bytestowrite -= byteswritten)
	{
		byteswritten = write(fd, bufp, bytestowrite);
		if ((byteswritten) == -1 && (errno != EINTR))
			return -1;
		if (byteswritten == -1)
			byteswritten = 0;
		totalbytes += byteswritten;
	}
	return totalbytes;
}

ssize_t r_read(int fd, void *buf, size_t size)
{
	ssize_t retval = 0;
	while (retval = read(fd, buf, size), retval == -1 && errno == EINTR)
		;
	return retval;
}

int r_close(int fildes)
{
	int retval = 0;
	while (retval = close(fildes), retval == -1 && errno == EINTR)
		;
	return retval;
}

int r_open2(const char *path, int oflag)
{
	int retval = 0;
	while (retval = open(path, oflag), retval == -1 && errno == EINTR)
		;
	return retval;
}

int r_mkfifo(const char *path, int oflag)
{
	int retval = 0;
	while (retval = mkfifo(path, oflag), retval == -1 && (errno == EINTR))
		;
	return retval;
}