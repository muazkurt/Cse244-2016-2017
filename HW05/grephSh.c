/* kaç thread oluştuğunu array içinde tut

 file detaylarını array ile döndür

 Aynı anda çalışanları bulmak için

		 mutex kilitle

		counter arttır

		full ile ilgili arkadaş < counter

		 counter = full

		lock aç

		 işlemler

		 işlemler

		lock

			counter azalt

		 unlock
*/
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
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SEM_NAME_LOG "/logsem"
#define SEM_NAME_COUNT "/countsem"
#define LOG_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LOG_MODES (O_WRONLY | O_APPEND)
#define FOUNDABLE_MAX 1024
#define MAX_THRADS 32
#define MAX_SUB_DIR 32
/*#define NAME_MAX 1024
#define PATH_MAX 1024*/
#define MILLION 1000000L
#define S_MEM_KEY 99999
#define M_QUE_KEY 99998
#define BUFFLEN 100


typedef struct
	{
		char			*filename,
							*search_string;

		sem_t			*log_sem,
							*counter_sem;
	} search_file;

typedef struct
	{
		int				total_string,
							total_lines,
							paralel_threads,
							doneflag;
	} return_value;

typedef struct
	{
		int 			total_string_main,
							subdir_members_main,
							total_files_main,
							total_lines_main,
							total_threads_main,
							error_detect;
	} main_return_val;

typedef struct
	{
		long 									mtype;
		main_return_val 			info;
	} msg_buffer;

int logfile = 0;



static sig_atomic_t 					currentworking = 0;

static pthread_once_t 				lockerror = PTHREAD_ONCE_INIT;

static pthread_rwlock_t 			logging;


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
				perror("Failed to open_logfile ");
				return -1;
			}
		return 0;
	}

int close_logfile(void)
	{
		if(close(logfile) == -1)
			{
				perror("Failed to close_logfile: ");
				return -1;
			}
			return 0;
	}

int log_all(char **index, int stack)
	{
		int 			i = 0,
							j = 0;
		while(pthread_rwlock_wrlock(&logging) != 0);
		for(i = 0; i < stack; ++i)
			for(j = 0; index[i][j] != 0; ++j)
				if(write(logfile, &index[i][j], sizeof(index[i][j])) <= 0)
					return -1;
		while(pthread_rwlock_unlock(&logging) != 0);
		return 0;
	}


/*old*/
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
				int 		count_file = 0,
								same_count = 1,
								error = 0;

				char		tmp_char = 0;

				off_t		pos = 0;

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
/*NEW*/

void *search_file_function(void *args)
	{
	/*Defining varibales*/
		char						tmp_char = 0,
										tmp_logging_index[256],
										*logging_index[FOUNDABLE_MAX];

		int							row = 1, /*Satır*/
										column = 1,/*Sütun*/
										find_counter = 0,
										read_count = 0,
										file_dest = 0,
										string_len = 0,
										paralel_counter = 0,
										maxworker = 0,
										free_count = 0,
										smemory = 0;

		search_file 		*names;

		return_value 		*file_info;

		key_t new_key_sm;

		names = (search_file *)args;
	/*Take From Shared Memory*/
		if((new_key_sm = ftok(names->filename, 'a')) == -1)
			{
				perror("Ftok in function");
				_exit(2);
			}
		if((smemory = shmget(new_key_sm, sizeof(return_value *), 0666)) == -1)
			{
		      perror("shmget in function");
		      _exit(2);
		    }
		if((file_info = (return_value *)shmat(smemory, NULL, 0)) == (void *)-1)
			{
				perror("shmat in function");
				_exit(2);
			}
	/*	file_info->doneflag = 0;*/
	/*open file*/
		file_dest = open(names->filename, O_RDONLY);
		lseek(file_dest, 0, SEEK_SET);
	/*Paralel Threads*/
		while(sem_wait(names->counter_sem) == -1);
		paralel_counter = addcount();
		if(paralel_counter > maxworker)
			maxworker = paralel_counter;
		while(sem_post(names->counter_sem) == -1);
	/*Find and keep for logging*/
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
				else if (tmp_char == names->search_string[0])
					{
						if (doesStringFound(file_dest, names->search_string) == 1)
							{
								++find_counter;
								string_len = snprintf(tmp_logging_index, 256, "%ld %ld %s : [%d, %d] %s first character is found.\n",
													(long)getpid(), (long)pthread_self(), names->filename, row, column, names->search_string);
								logging_index[find_counter - 1] = (char *)malloc((string_len + 1) * sizeof(char));
								snprintf(logging_index[find_counter - 1], string_len + 1, "%s", tmp_logging_index);
							}
						else ;
					}
				++column;
			}
	/*Lock and Log*/
		while(sem_wait(names->log_sem) == -1);
		if(log_all(logging_index, find_counter) != 0);
		while(sem_post(names->log_sem) == -1);
	/*Paralel Threads*/
		while(sem_wait(names->counter_sem) == -1);
		remcount();
		while(sem_post(names->counter_sem) == -1);
	/*exit*/
		for(free_count = 0; free_count < find_counter; ++free_count)
			{
				free(logging_index[free_count]);
				logging_index[free_count] = NULL;
			}
		file_info->total_string = find_counter;
		file_info->total_lines = row;
		file_info->paralel_threads = paralel_counter;
		file_info->doneflag = 1;
		if(shmdt(file_info) == -1)
			perror("shmdt in function");
		_exit(2);
	/*End of function*/
	}

int ListDirfunction(const char *input_string, const char *dir_name)
	{
	/*Defining varibales*/
		struct dirent				*dir_index = NULL;

		DIR 								*search_dir = NULL;

		int									family_members = 0,
												subdir_members = 0,
												join_count = 0,
												stack_counter = 0,
												tmp_int[5],
												smemory[MAX_THRADS],
												msgque[MAX_SUB_DIR];

		char								address[PATH_MAX];

		pthread_t 					thread_family[MAX_THRADS];

		pid_t								subdir[MAX_SUB_DIR];

		search_file					args[MAX_THRADS];

		return_value				*file_info = NULL;

		main_return_val			*dir_info = NULL;

		sem_t								*sem_logger = NULL,
												*sem_counter = NULL;

		msg_buffer					subdir_info;

		key_t								new_key_mq,
												new_key_sm;
	/*Creating key*/
		if((new_key_mq = ftok(dir_name, 'a')) == -1)
			{
				perror("Ftok mq in dir");
				return -1;
			}
	/*Creating Mesage Queue*/
		if((msgque[subdir_members] = msgget(new_key_mq, 0666)) == -1) {
			perror("msgget in dir");
			return -1;
		}
	/*Casting to 0 first results*/
			dir_info = (main_return_val *)malloc(sizeof(main_return_val));
			dir_info->total_string_main = 0;
			dir_info->subdir_members_main = 0;
			dir_info->total_files_main = 0;
			dir_info->total_lines_main = 0;
			dir_info->total_threads_main = 0;
	/*Semaphore defines*/
		if((sem_logger = sem_open(SEM_NAME_LOG, O_CREAT | O_EXCL, (mode_t)LOG_PERMS, 1)) == SEM_FAILED)
			{
				if(errno == EEXIST)
					sem_logger = sem_open(SEM_NAME_LOG, 0);
				else return -1;
			}

		if((sem_counter = sem_open(SEM_NAME_COUNT, O_CREAT | O_EXCL, (mode_t)LOG_PERMS, 1)) == SEM_FAILED)
			{
				if(errno == EEXIST)
					sem_counter = sem_open(SEM_NAME_COUNT, 0);
				else return -1;
			}
	/*Opening Directory*/
		if ((search_dir = opendir(dir_name)) == NULL)
			{
				fprintf(stderr, "Failed to opening directory %s", dir_name);
				return -1;
			}
	/*Reading all files and sub-directories*/
		while ((dir_index = readdir(search_dir)) != NULL) {
				sprintf(address, "%s/%s", dir_name, dir_index->d_name);
				/*Checking if regular file*/
					if (isRegularFile(address))
						{
							/*Igonre conditions*/
								if((strncmp(dir_index->d_name, "log.log", sizeof("log.log") - 1) == 0) ||
									(strncmp(dir_index->d_name, "grephSh", sizeof("grephSh") - 1) == 0));
							/*Keeping file infos for search*/
								else
									{
										/*Creating space for file info*/
											args[family_members].filename = (char *)malloc(sizeof(address) + 1);
											args[family_members].search_string = (char *)malloc(sizeof(input_string) + 1);
										/*Filling the space*/
											snprintf(args[family_members].filename, sizeof(address), "%s", address);
											snprintf(args[family_members].search_string, sizeof(input_string), "%s", input_string);
											args[family_members].log_sem = sem_logger;
											args[family_members].counter_sem = sem_counter;
										/*Creating Shared Memory*/
											if((new_key_sm = ftok(args[family_members].filename, 'a')) == -1)
												{
													perror("Ftok shared memory in dir");
													return -1;
												}
											if((smemory[family_members] = shmget(new_key_sm, sizeof(file_info),  IPC_CREAT | 0666)) == -1)
												{
													perror("shmget in dir ");
													return -1;
												}
										++family_members;
									}
						}
				/*Checking if directory*/
					else if (isdirectory(address))
						{
							/*Ignoring conditions*/
								if ((strcmp(dir_index->d_name, ".") == 0) || (strcmp(dir_index->d_name, "..") == 0));
							/*Searching directory*/
								else
									{
										/*Creating Mesage Queue for sub-directory*/
											if((new_key_mq = ftok(address, 'a')) == -1)
												{
													perror("Ftok mq in subdir");
													return -1;
												}
											if((msgque[subdir_members + 1] = msgget(new_key_mq, IPC_CREAT | 0666)) == -1)
												{
													perror("msgget for subdir");
													return -1;
												}
										/*Creating new process and storing it's pid for sub-directory*/
											if ((subdir[subdir_members] = fork()) == -1)
												perror("Failed to fork for Directory!");
											if(subdir[subdir_members] == 0)
												{
													ListDirfunction(input_string, address);
													exit(0);
												}
										++subdir_members;
									}
						}
				/*Other conditions*/
					else ;
			}
	/*Filling result of total file/sub-directory for this directory*/
		dir_info->subdir_members_main += subdir_members;
		dir_info->total_files_main += family_members;
	/*Starting proceses by threads for all files in this directory*/
		while(join_count < family_members)
			{
				/*Creating Shared Memory object*/
			 		if((file_info = (return_value *)shmat(smemory[join_count], NULL, 0)) == (void *) -1)
						{
							perror("shmat dir");
							return -1;
						}
				file_info->doneflag = 0;
				pthread_create(&thread_family[join_count], NULL, search_file_function, &args[join_count]);
				while(!file_info->doneflag);
				dir_info->total_string_main += file_info->total_string;
				dir_info->total_lines_main += file_info->total_lines;
				if((tmp_int[0] = file_info->paralel_threads) > dir_info->total_threads_main)
					dir_info->total_threads_main = tmp_int[0];
				/*Emptying Shared Memory object*/
					if(shmdt(file_info) == -1)
			      		perror("shmdt dir");
				/*Deleting Shared Memory object*/
					if(shmctl(smemory[join_count], IPC_RMID, NULL) == -1)
			     		perror("shmctl for removal: ");
				++join_count;
			}
	join_count = 0;

	/*Checking if there is/are sub-directories*/
		if(subdir_members > 0)
			{
				/*Waiting for their job to finish*/
					while(join_count < subdir_members)
						{
							wait(NULL);
							++join_count;
						}
				/*Taking their results and updating own result by them*/
					while(stack_counter < subdir_members)
						{
							subdir[stack_counter] = 0;
							if(msgrcv(msgque[stack_counter + 1], (void *)&subdir_info, sizeof(msg_buffer), 1, 0) == -1)
								perror("Msgrcv subdir");
							dir_info->total_string_main += subdir_info.info.total_string_main;
							dir_info->subdir_members_main += subdir_info.info.subdir_members_main;
							dir_info->total_files_main += subdir_info.info.total_files_main;
							dir_info->total_lines_main += subdir_info.info.total_lines_main;
							if(subdir_info.info.total_threads_main > dir_info->total_threads_main)
								dir_info->total_threads_main = subdir_info.info.total_threads_main;
							if(msgctl(msgque[stack_counter + 1], IPC_RMID, 0) == -1)
								perror("Msgctl subdir");
							++stack_counter;
						}
				/**/
			}
	/*Filling message info */
			subdir_info.info.total_string_main = dir_info->total_string_main;
			subdir_info.info.subdir_members_main = dir_info->subdir_members_main;
			subdir_info.info.total_files_main = dir_info->total_files_main;
			subdir_info.info.total_lines_main = dir_info->total_lines_main;
			subdir_info.info.total_threads_main = dir_info->total_threads_main;
			subdir_info.mtype = 1;
	/*Sending message*/
		if(msgsnd(msgque[0], (void *)&subdir_info, sizeof(msg_buffer), 0) == -1)
			perror("Msgsnd subdir");
	/*Closing semaphores*/
			sem_close(sem_logger);
			sem_close(sem_counter);
	/*Closing Directory*/
			closedir(search_dir);
			return 0;
	/**/
	}

int main(int argc, char **argv)
{
	/*Defining varibales*/
		int							i = 0,
										msg_no = 0,
										tmp_int[5];

		pid_t						childpid = 0;

		char						found_times[FOUNDABLE_MAX];

		long						timedif = 0;

		struct timeval	tpstart,
										tpend;

		msg_buffer			function_result;

		key_t 					new_key_mq;
	if(argc != 3)
		{
			fprintf(stderr, "%s \"string\" <dir_name>\n", argv[0]);
			return -1;
		}
	/*Get time of day*/
		gettimeofday(&tpstart, NULL);
	/*Making String lovercase*/
		argv[1] = makeStrLover(argv[1]);
	/*Create/open Log file*/
		if(create_logfile() != 0)																					return -1;
		if(open_logfile() != 0)																						return -1;
	/*Init read/write lock*/
		init_lock_once();
	/*Create Message Queue*/
		if((new_key_mq = ftok(argv[2], 'a')) == -1)
			{
				perror("Ftok mq in main");
				return -1;
			}
		while((msg_no = msgget(new_key_mq, (IPC_CREAT | IPC_EXCL) | 0666)) == -1) {
				perror("msgget in main");
				return -1;
		}
	/*Searching job*/
		if((childpid = fork()) == -1)
			{
				perror("Failed to main Fork");
				return -1;
			}
		if(childpid == 0)
			{
				ListDirfunction(argv[1], argv[2]);
				exit(0);
			}
		while(waitpid(childpid, NULL, 0) != childpid);
	/*Taking the mesage from queue*/
		if(msgrcv(msg_no, (void *)&function_result, sizeof(msg_buffer), 1, 0) == -1)
			perror("Msgrcv main");
	/*Delete Mesage Queue*/
		if(msgctl(msg_no, IPC_RMID, 0) == -1)
			perror("Msgctl main");
	/*Parsing Mesage*/
		tmp_int[0] = function_result.info.total_string_main;
		tmp_int[1] = function_result.info.subdir_members_main;
		tmp_int[2] = function_result.info.total_files_main;
		tmp_int[3] = function_result.info.total_lines_main;
		tmp_int[4] = function_result.info.total_threads_main;
	/*Making printable log file item*/
		snprintf(found_times, FOUNDABLE_MAX, "%d %s were found in total.\n", tmp_int[0], argv[1]);
	/*Writing log file*/
		for(i = 0; found_times[i] != 0; ++i)
			write(logfile, &found_times[i], sizeof(char));
	/*Closing log file*/
		if(close_logfile() != 0)
			return -1;
	/*Get time of day*/
		gettimeofday(&tpend, NULL);
	/*Time dif.*/
		timedif = MILLION * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
	/*Printing the printable*/
		fprintf(stderr, "Total number of strings found: \t\t%d\nNumber of directories searched:\t\t%d\nNumber of files seached: \t\t%d\nNumber of lines searched:\t\t%d\nNumber of search threadss created\t%d\nMax # of threads running concurrently:\t%d\nTotal run time, in milisecounds:\t%ld\n",
			tmp_int[0], tmp_int[1], tmp_int[2], tmp_int[3], tmp_int[2], tmp_int[4], timedif);
	/*Removing Semaphores*/
		sem_unlink(SEM_NAME_LOG);
		sem_unlink(SEM_NAME_COUNT);
	return 0;
}
