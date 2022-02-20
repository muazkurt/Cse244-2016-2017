#include "grepTH.h"
int logfile = 0;
int main(int argc, char **argv)
{
	int i = 0;
	char found_times[FOUNDABLE_MAX];
	long timedif = 0;
	struct timeval tpstart, tpend;
	main_return_val *function_result;
	if (argc != 3)
	{
		fprintf(stderr, "%s \"string\" <filename>\n", argv[0]);
		return -1;
	}
	gettimeofday(&tpstart, NULL);
	argv[1] = makeStrLover(argv[1]);
	create_logfile();
	open_logfile();
	init_lock_once();

	function_result = ListDirfunction(argv[1], argv[2]);
	gettimeofday(&tpend, NULL);
	snprintf(found_times, FOUNDABLE_MAX, "%d %s were found in total.\n", function_result->total_string_main, argv[1]);
	timedif = MILLION * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
	fprintf(stderr, "Total number of strings found: \t\t%d\nNumber of directories searched:\t\t%d\nNumber of files seached: \t\t%d\nNumber of lines searched:\t\t%d\nNumber of search threadss created\t%d\nMax # of threads running concurrently:\t%d\nTotal run time, in milisecounds:\t%ld\n",
			function_result->total_string_main, function_result->subdir_members_main, function_result->total_files_main, function_result->total_lines_main, function_result->total_files_main, function_result->total_threads_main, timedif);
	for (i = 0; found_times[i] != 0; ++i)
		write(logfile, &found_times[i], sizeof(char));
	sem_unlink(SEM_NAME_LOG);
	sem_unlink(SEM_NAME_COUNT);
}