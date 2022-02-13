#include "Listdir.h"
/*
	Listdir
	Takes three arguments while openning.
	Listdir "search string" <DirName>
	Goes inside to the Directory and
			searches the string in every files included this directory.
		If directory has any directory inside,
			Opens these directory/ies and does same thing them too.
	While searching, every found part will bi printed to log.log file.

	When the work finish, log.log file's end will be printen with
	How much found happened.
*/
int main(int argc, char * argv[])
{
	DIR *OpenedDir;
	FILE *LogFile;
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
	LogFile = fopen("log.log", "w+");
	ListDirfunction(argv[1], argv[2], LogFile);
	fseek(LogFile, 0, SEEK_SET);
	fprintf(LogFile, "%d %s were found in total.\n", HowManyFound(LogFile), argv[1]);
	fclose(LogFile);
	return 0;
}