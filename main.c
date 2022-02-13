#include "List.h"
int main(int argv, char * argc[])
{
	FILE *FileWillSearchen;
	/*Usage part of the tool.*/
	if (argv == 1)
	{
		fprintf(stderr, "This tool takes a string for query and a filename,\n");
		fprintf(stderr, "searches the String inside of the given filename.\n");
		fprintf(stderr, "Each time if it finds the query, prints where it was found as:\n");
		fprintf(stderr, "->Times of Found: ~, Row: ~, Column: ~\n");
		fprintf(stderr, "Usage of the 'List': \n");
		fprintf(stderr, "  ->%s \"String\" <Filename>\n", argc[0]);
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
		FileWillSearchen = fopen(argc[2], "r");
		if (FileWillSearchen != NULL)
			searchOpenedFile(FileWillSearchen, makeStrLover(argc[1]));
		else
		{
			fprintf(stderr, "Error while openning file.\n");
			return EXIT_FAILURE;
		}
	}
	fclose(FileWillSearchen);
	return EXIT_SUCCESS;
}