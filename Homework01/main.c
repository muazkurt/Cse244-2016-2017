#include "List.h"
int main(int argv, char * argc[])
{
	int fd;
	/*Usage part of the tool.*/
	if (argv == 1)
	{
		fprintf(stderr, "Usage of the 'List': \n");
		fprintf(stderr, "\t->%s \"String\" <Filename>\n", argc[0]);
		fprintf(stderr, "\t->This tool takes a String and File Name,\n");
		fprintf(stderr, "\t\tsearches the String inside of the taken File.\n");
		fprintf(stderr, "\t->Each time if found it, prints where it found\n");
		fprintf(stderr, "\t\tlike;\n\t->Times of Found: ~, Row: ~, Column: ~\n");
		return -1;
	}
	/*Error situation.*/
	else if (argv != 3)
	{
		fprintf(stderr, "Invalid input for tool.\n");
		fprintf(stderr, "Please use like this: \n");
		fprintf(stderr, "\t->List \"String\" <Filename>\n");
		return -1;
	}
	/*Working part of the tool.*/
	else
	{
		printf("%s, %s, %s\n", argc[0], argc[1], argc[2]);
		
		if ((fd = open(argc[2], O_RDONLY)) == -1)
		{
			fprintf(stderr, "Error while openning file.\n");
			return -1;
		}
		searchfile(fd, makeStrLover(argc[1]));
		close(fd);
	}
	return 0;
}