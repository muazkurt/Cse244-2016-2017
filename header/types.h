#ifndef TYPES_H
#define TYPES_H
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#ifndef PATH_MAX
#define PATH_MAX 256
#endif
#ifndef NAME_MAX
#define NAME_MAX 256
#endif

#define FOUNDABLE_MAX 8192
#define BUFFER_SIZE   512
#define FAILURE_CASE  -1


typedef struct return_value_t {
    int occurences, line_count;
} return_value;

typedef struct parmeters_t {
    return_value result;
    int logFile;
    const char *base_path, *query;
    char referance_path[PATH_MAX];
    sem_t *log_sem, *counter_sem;
    pthread_rwlock_t logging;
} parameters;

typedef struct {
    int total_occurences, subdir_members_main, total_files_main, total_line_count,
        total_threads_main, error_detect;
} main_return_val;

#endif