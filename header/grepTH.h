// kaç thread oluştuğunu array içinde tut

// file detaylarını array ile döndür

// Aynı anda çalışanları bulmak için

// mutex kilitle

// counter arttır

// full ile ilgili arkadaş < counter

// counter = full

// lock aç

// işlemler

// işlemler

// lock

// counter azalt

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
	char filename[NAME_MAX],
		search_string[NAME_MAX];
	sem_t *log_sem,
		*counter_sem;
} search_file;

typedef struct
{
	int total_string,
		total_lines,
		paralel_threads;
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

static void init_rwlcog(void);
int init_lock_once(void);
int addcount(void);
void remcount(void);
int search_pid(pid_t *stack, pid_t child_id, int stack_size);
int create_logfile(void);
int open_logfile(void);
int log_all(char **index, int stack);

/// old
char makeCharLover(char inputChar);
char *makeStrLover(char *inputStr);
int isRegularFile(char *path);
int isdirectory(char *path);
int doesStringFound(int openedFile, const char stringtaken[]);
void *search_file_function(void *args);
main_return_val *ListDirfunction(const char *input_string, const char *dir_name);
