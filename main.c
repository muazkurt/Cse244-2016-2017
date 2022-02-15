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
int main(int argc, char *argv[])
{
	FILE *LogFile;
	int foundTimes = 0, fifoInt = 0;
	if (argc != 3)
	{
		fprintf(stderr, "This tool takes a string for query and directory name,\n"
						"searches the String inside of the givenen directory name.\n"
						"Each time if it finds the query, prints where it found int log.log file as:\n"
						"-><Filename>:  [~,~]  \"string\" first character is found.\n"
						"Usage: \n"
						"  ->%s \"String\" <Directoryname>\n",
				argv[0]);
		return EXIT_FAILURE;
	}
	return listDir(argv[1], argv[2]);
}