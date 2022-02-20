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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SEM_NAME_LOG   "/logsem"
#define SEM_NAME_COUNT "/countsem"
#define LOG_PERMS	   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LOG_MODES	   (O_WRONLY | O_APPEND)
#define MAX_THRADS	   32
#define MAX_SUB_DIR	   32
#define MILLION		   1000000L


static void init_rwlcog(void);
int			init_lock_once(void);
int			addcount(void);
void		remcount(void);
int			search_pid(pid_t *stack, pid_t child_id, int stack_size);
int			create_logfile(void);
int			open_logfile(void);
int			log_all(char **index, int stack);