// kaç thread oluştuğunu array içinde tut

// file detaylarını array ile döndür

// Aynı anda çalışanları bulmak için

		// mutex kilitle

		//counter arttır

		//full ile ilgili arkadaş < counter

		// counter = full

		//lock aç

		// işlemler

		// işlemler

		//lock 

			//counter azalt

		// unlock

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

#define SEM_NAME_LOG "/logsem"
#define SEM_NAME_COUNT "/countsem"
#define LOG_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LOG_MODES (O_WRONLY | O_APPEND)
#define FOUNDABLE_MAX 1024
#define MAX_THRADS 32
#define MAX_SUB_DIR 32
#define MILLION 1000000L


typedef struct
	{
		char
				filename[NAME_MAX],
				search_string[NAME_MAX];
		sem_t 
				*log_sem,
				*counter_sem;
	} search_file;

typedef struct
	{
		int total_string,
			total_lines,
			paralel_threads;
			//cascade_threads,
			//created_threads;
	} return_value;

typedef struct 
	{
		int total_string_main,
			subdir_members_main,
			total_files_main,
			total_lines_main,
			total_threads_main,
			error_detect;
	} main_return_val;

int logfile = 0;



static sig_atomic_t currentworking = 0;
			
static pthread_once_t lockerror = PTHREAD_ONCE_INIT;

static pthread_rwlock_t logging;


static void init_rwlcog(void)
	{
		lockerror = pthread_rwlock_init(&logging, NULL);
	}

int init_lock_once(void)
	{
		if(pthread_once(&lockerror, init_rwlcog))
			lockerror = EINVAL;
		return lockerror;
	}




int addcount(void)
	{
		++currentworking;
		return currentworking;
	}

void remcount(void)
	{
		--currentworking;
	}

int search_pid(pid_t *stack, pid_t child_id, int stack_size)
	{
		int i = 0;
		for(i = 0; i < stack_size; ++i)
		{
			if(stack[i] == child_id)
				return i;
		}
		return -1;
	}

int create_logfile(void)
	{
		while(open("log.log", O_CREAT | O_EXCL, LOG_PERMS) <= 0)
		{
			if(errno == EEXIST)
			{
				if(unlink("log.log") == -1)
					perror("Failed to refresh log");
			}
			else
				{
					perror("Failed to create_logfile");
					return -1;
				}
		}
		return 0;
	}

int open_logfile(void)
	{
		if((logfile = open("log.log", LOG_MODES)) <= 0)
			{
				perror("Failed to open_logfile");
				return -1;
			}
		return 0;
	}

int log_all(char **index, int stack)
	{
		int i = 0, j = 0;
		while(pthread_rwlock_wrlock(&logging) != 0);
		for(i = 0; i < stack; ++i)
			for(j = 0; index[i][j] != 0; ++j)
				if(write(logfile, &index[i][j], sizeof(index[i][j])) <= 0)
					return -1;
		while(pthread_rwlock_unlock(&logging) != 0);
		return 0;
	}


///old
	char makeCharLover(char inputChar)
		{
			if (inputChar >= 'A' && inputChar <= 'Z')
				inputChar += 'a' - 'A';
			else;
			return inputChar;
		}

	char *makeStrLover(char *inputStr)
		{
			int charCounter = 0;
			while (inputStr[charCounter] != 0)
				{
					inputStr[charCounter] = makeCharLover(inputStr[charCounter]);
					++charCounter;
				}
			return inputStr;
		}

	int isRegularFile(char *path)
		{
			struct stat statbuf;
			if (stat(path, &statbuf) == -1)
				return 0;
			else
				return S_ISREG(statbuf.st_mode);
		}

	int isdirectory(char *path)
		{
			struct stat statbuf;
			if (stat(path, &statbuf) == -1)
				return 0;
			else
				return S_ISDIR(statbuf.st_mode);
		}


	int doesStringFound(int openedFile, const char stringtaken[])
		{
			int count_file = 0, same_count = 1, error = 0;
			char tmp_char = 0;
			off_t pos = 0;
			if((pos = lseek(openedFile, 0, SEEK_CUR)) == -1)
				perror("Failed to get location");
			while ((stringtaken[same_count] != '\0') && (error != 1))
				{
					if(read(openedFile, &tmp_char, sizeof(tmp_char)) <= 0)
						{
							error = 1;
							break;
						}
					tmp_char = makeCharLover(tmp_char);
					++count_file;
					if (tmp_char == '\n' || tmp_char == '\t' || tmp_char == ' ');
					else if (tmp_char == stringtaken[same_count])
						++same_count;
					else if (tmp_char != stringtaken[same_count])
						error = 1;
				}
			if(lseek(openedFile, pos + 1, SEEK_SET) != pos + 1)
				perror("Failed to set location");
			if (error == 0)
				return 1;
			return 0;
		}


void *search_file_function(void *args)
	{
		char
			tmp_char = 0,
			tmp_logging_index[256],
			string_index[NAME_MAX],
			*logging_index[FOUNDABLE_MAX];
		int 
			row = 1, /*Satır*/
			column = 1,/*Sütun*/
			find_counter = 0,
			read_count = 0,
			file_dest = 0,
			string_len = 0,
			paralel_counter = 0,
			maxworker = 0,
			free_count = 0;

		search_file *names;
		
		return_value *file_info;

	//open file
		names = (search_file *)args;
		file_info = (return_value *)malloc(sizeof(return_value));

		snprintf(string_index, NAME_MAX, "%s", names->search_string);


	//	logging_index = (char **)malloc(FOUNDABLE_MAX * sizeof(char *));

		file_dest = open(names->filename, O_RDONLY);
		lseek(file_dest, 0, SEEK_SET);
	//Semaphore
		while(sem_wait(names->counter_sem) == -1);
		paralel_counter = addcount();
		if(paralel_counter > maxworker)
			maxworker = paralel_counter;
		while(sem_post(names->counter_sem) == -1);


	//Works Well
		while((read_count = read(file_dest, &tmp_char, sizeof(tmp_char))) > 0)
			{
				tmp_char = makeCharLover(tmp_char);
					/*
						Eğer yeni satır karakteri gelirse,
						rowinLine değişkeni arttırılıp,
						column 1 ile değiştiriliyor.
					*/
				if (tmp_char == '\n')
					{
						++row;
						column = 0;
					}
					/*
					Eğer okunan karakter,
					aranan karakter ile aynı ise;
					Aranan kelimenin bütünü var mı diye
					kontrol etmeye gidiyor.
					*/
				else if (tmp_char == string_index[0])
					{
						if (doesStringFound(file_dest, string_index) == 1)
							{
								++find_counter;
								string_len = snprintf(tmp_logging_index, 256, "%ld %ld %s : [%d, %d] %s first character is found.\n",
													(long)getpid(), (long)pthread_self(), names->filename, row, column, string_index);
								logging_index[find_counter - 1] = (char *)malloc((string_len + 1) * sizeof(char));
								snprintf(logging_index[find_counter - 1], string_len + 1, "%s", tmp_logging_index);
							}
						else ;
					}
				++column;
			}
	//semaphore
		while(sem_wait(names->log_sem) == -1);
		if(log_all(logging_index, find_counter) != 0);
		while(sem_post(names->log_sem) == -1);

	//semaphore	unnamed
		while(sem_wait(names->counter_sem) == -1);
		remcount();
		while(sem_post(names->counter_sem) == -1);
	//exit
		for(free_count = 0; free_count < find_counter; ++free_count)
			free(logging_index[free_count]);
	//	free(logging_index);

		file_info->total_string = find_counter;
		file_info->total_lines = row;
		file_info->paralel_threads = paralel_counter;
		//pthread_exit(file_info);
		//free(file_info);
		return (void *)file_info;

	}

main_return_val *ListDirfunction(const char *input_string, const char *dir_name)
	{
		struct dirent *dir_index = NULL;
		
		DIR *search_dir = NULL;
		
		int 
			family_members = 0,
			subdir_members = 0,
			join_count = 0,
			total_lines = 0,
			total_string = 0,
			subdir_fifo_fp[MAX_SUB_DIR],
			stack_counter = 0,
			maxthread,
			tmp_int = 0;
		
		char 
			address[PATH_MAX],
			subdir_fifo[MAX_SUB_DIR][PATH_MAX];
		
		pthread_t thread_family[MAX_THRADS];

		pid_t subdir[MAX_SUB_DIR], tmp_pid;

		search_file args[MAX_THRADS];

		return_value *file_info;

		main_return_val *dir_info;

		sem_t *sem_logger, *sem_counter;
		

		//imports
	/*		args = (search_file *)malloc(MAX_THRADS * sizeof(search_file));

			thread_family = (pthread_t *)malloc(MAX_THRADS * sizeof(pthread_t));
			
			subdir = (pid_t *)malloc(MAX_SUB_DIR * sizeof(pid_t));*/

			dir_info = (main_return_val *)malloc(sizeof(main_return_val));
			dir_info->total_string_main = 0;
			dir_info->subdir_members_main = 0;
			dir_info->total_files_main = 0;
			dir_info->total_lines_main = 0;
			dir_info->total_threads_main = 0;

		//semaphore
				
			if((sem_logger = sem_open(SEM_NAME_LOG, O_CREAT | O_EXCL, (mode_t)LOG_PERMS, 1)) == SEM_FAILED)
				{
					if(errno == EEXIST)
						sem_logger = sem_open(SEM_NAME_LOG, 0);
					else exit(1);
				}

			if((sem_counter = sem_open(SEM_NAME_COUNT, O_CREAT | O_EXCL, (mode_t)LOG_PERMS, 1)) == SEM_FAILED)
				{
					if(errno == EEXIST)
						sem_counter = sem_open(SEM_NAME_COUNT, 0);
					else exit(1);
				}
			
		//
			if ((search_dir = opendir(dir_name)) == NULL)
				{
					fprintf(stderr, "Failed to opening directory %s", dir_name);
					return dir_info;
				}
			
			while ((dir_index = readdir(search_dir)) != NULL)
				{
					sprintf(address, "%s/%s", dir_name, dir_index->d_name);
					//File ise
						if (isRegularFile(address))
							{
								if ((strncmp(dir_index->d_name, "log.log", sizeof("log.log") - 1) == 0) ||
									(strncmp(dir_index->d_name, "fuck.c", sizeof("fuck.c") - 1) == 0));
								else
									//file arama işlemleri
									{
										snprintf(args[family_members].filename, sizeof(address), "%s", address);
										snprintf(args[family_members].search_string, sizeof(input_string), "%s", input_string);
										args[family_members].log_sem = sem_logger;
										args[family_members].counter_sem = sem_counter;
										++family_members;
									}
							}
					//Directory ise.
						else if (isdirectory(address))
						{
							if ((strcmp(dir_index->d_name, ".") == 0) || (strcmp(dir_index->d_name, "..") == 0));
							else
								{
									sprintf(subdir_fifo[subdir_members], "%d", tmp_int);
									while(mkfifo(subdir_fifo[subdir_members], S_IRWXU | S_IRGRP | S_IROTH) == -1)
										{	
											if(errno == EEXIST)
											{
												++tmp_int;
												sprintf(subdir_fifo[subdir_members], "%d", tmp_int);
											}
										}
									subdir_fifo_fp[subdir_members] = open(subdir_fifo[subdir_members], O_RDWR);
									if ((subdir[subdir_members] = fork()) == -1)
										perror("Failed to fork for Directory!");
									if(subdir[subdir_members] == 0)
										{
											dir_info = ListDirfunction(input_string, address);
											write(subdir_fifo_fp[subdir_members], &dir_info->total_string_main, sizeof(int));
											dir_info->total_string_main = 0;
											write(subdir_fifo_fp[subdir_members], &dir_info->subdir_members_main, sizeof(int));
											dir_info->subdir_members_main = 0;
											write(subdir_fifo_fp[subdir_members], &dir_info->total_files_main, sizeof(int));
											dir_info->total_files_main = 0;
											write(subdir_fifo_fp[subdir_members], &dir_info->total_lines_main, sizeof(int));
											dir_info->total_lines_main = 0;
											write(subdir_fifo_fp[subdir_members], &dir_info->total_threads_main, sizeof(int));
											dir_info->total_threads_main = 0;
											close(subdir_fifo_fp[subdir_members]);
											free(dir_info);
											exit(1);
										}
									++subdir_members;
								}
						}
					else ;
				}
			
			dir_info->subdir_members_main += subdir_members;
			dir_info->total_files_main += family_members;
			
			while(join_count < family_members)
				{
					pthread_create(&thread_family[join_count], NULL, search_file_function, &args[join_count]);
					++join_count;
				}
			join_count = 0;
			
			while(join_count < family_members)
				{
					pthread_join(thread_family[join_count], (void **)&file_info);
			//		pthread_detach(thread_family[join_count]);
					++join_count;
					dir_info->total_string_main += file_info->total_string;
					dir_info->total_lines_main += file_info->total_lines;
					if((tmp_int = file_info->paralel_threads) > dir_info->total_threads_main)
						dir_info->total_threads_main = tmp_int;
				}
			join_count = 0;
			
			if(subdir_members > 0)
				{
				
					while(join_count < subdir_members)
						{
							tmp_pid = wait(NULL);
							++join_count;
						}

					while(stack_counter < subdir_members)
					{
						subdir[stack_counter] = 0;
						read(subdir_fifo_fp[stack_counter], &tmp_int, sizeof(int));
						dir_info->total_string_main += tmp_int;
						read(subdir_fifo_fp[stack_counter], &tmp_int, sizeof(int));
						dir_info->subdir_members_main += tmp_int;
						read(subdir_fifo_fp[stack_counter], &tmp_int, sizeof(int));
						dir_info->total_files_main += tmp_int;
						read(subdir_fifo_fp[stack_counter], &tmp_int, sizeof(int));
						dir_info->total_lines_main += tmp_int;
						read(subdir_fifo_fp[stack_counter], &tmp_int, sizeof(int));
						if(tmp_int > dir_info->total_threads_main)
							dir_info->total_threads_main = tmp_int;
						close(subdir_fifo_fp[stack_counter]);
						if(unlink(subdir_fifo[stack_counter]));
						for(tmp_int = 0; subdir_fifo[stack_counter][tmp_int] != 0; ++tmp_int)
							subdir_fifo[stack_counter][tmp_int] = 0;
						++stack_counter;
					}
				}
			free(file_info);
			sem_close(sem_logger);
			sem_close(sem_counter);
			closedir(search_dir);
			return dir_info;
	}

int main(int argc, char **argv)
{
	int i = 0;
	char found_times[FOUNDABLE_MAX];
	long timedif = 0;
	struct timeval tpstart, tpend;
	main_return_val *function_result;
	if(argc != 3)
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
		function_result->total_string_main, function_result->subdir_members_main,function_result->total_files_main, function_result->total_lines_main, function_result->total_files_main, function_result->total_threads_main, timedif);
	for(i = 0; found_times[i] != 0; ++i)
		write(logfile, &found_times[i], sizeof(char));
	sem_unlink(SEM_NAME_LOG);
	sem_unlink(SEM_NAME_COUNT);
}