#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
	int namedPipe = 0;
	pid_t serverPid = 0;
	if(argc != 2)
	{
		return -1;
	}
	if((namedPipe = open(argv[1], O_RDONLY)) == -1)
		perror("Failed to open named pipe.");
	if(read(namedPipe, &serverPid, sizeof(serverPid)) == -1)
		perror("Failed to open server pid.");
	close(namedPipe);
	if((kill(serverPid, SIGINT) == -1) && errno == ESRCH)
		perror("There is no such pid.");
	while(1);
	return 0;
}