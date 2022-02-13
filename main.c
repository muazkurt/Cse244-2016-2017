#include "Listdir.h"
/*
	Listing directories using pipe and Fifo.
	Usage:
		->./exe "search.string" <dir.name>
	
	When the command taken,
		Opens the given named directory.
		Each time it finds a file,
			Sets a new pipeline for communicate child process of searching file.
		Each time if it finds a subdirect�ry,
			Sest a new Fifo named: 'found.directory.name-Fifo'
		The parent process sums the pipe values, and sub directories Fifo values.
		Then prints it upper (main function Fifo to communicate to job) Fifo and returns 0 as finished Correctly.
		Main fuction reads the Fifo, and prints it's value to log.log file.
*/
int main(int argc, char * argv[])
{
	DIR *OpenedDir;
	FILE *LogFile;
	int foundTimes = 0, fifoInt = 0;
	char buffer[FOUNDABLE_MAX + 1];
	if (argc != 3)
	{
		fprintf(stderr, "This tool takes a string for query and directory name,\n");
		fprintf(stderr, "searches the String inside of the givenen directory name.\n");
		fprintf(stderr, "Each time if it finds the query, prints where it found int log.log file as:\n");
		fprintf(stderr, "-><Filename>:  [~,~]  \"string\" first character is found.\n");
		fprintf(stderr, "Usage: \n");
		fprintf(stderr, "  ->%s \"String\" <Directoryname>\n", argv[0]);
		return -1;
	}
	if ( !(isdirectory(argv[2])))
	{
		fprintf(stderr, "%s is not a directory.\n", argv[2]);
		return -1;
	}
	while ((mkfifo("Fifo", FIFO_PERM) == -1) && (errno == EINTR));
	while(((fifoInt = open("./Fifo", O_RDWR)) == -1) && (errno == EINTR));
	LogFile = fopen("log.log", "w+");
	ListDirfunction(argv[1], argv[2], LogFile, fifoInt);
	r_read(fifoInt, buffer, FOUNDABLE_MAX);
	foundTimes += atoi(buffer);
	fseek(LogFile, 0, SEEK_END);
	fprintf(LogFile, "\n%d times %s found in %s directory.\n", foundTimes, argv[1], argv[2]);
	fclose(LogFile);
	while ((close(fifoInt) == -1) && (errno == EINTR));
	if(unlink("Fifo") == -1)
	{
		perror("Error to unlink Fifo");
		return -1;
	}
	return 0;
}