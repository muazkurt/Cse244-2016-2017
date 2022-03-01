/**
 * Todos:
 * -> Open the App
 * -> Get the filename
 * -> Get a query as a word or a letter to search in the given file.
 * -> Open the given filename for reading
 * -> For all lines into the given file
 *  -> Read a line
 *  -> If the query is found in the given line:
 *   -> Print the position of the foundation into a log file as "line number,
 *		column"
 *  -> Else:
 *  	-> If there are less letters to the end of the file than the given
 *			quuery:
 *    -> Close the file
 *    -> Quit the program
 * When reach the end of the file, close the file.
 * Clear the used variables.
 * Close the program.
 **/
#include "List.h"

#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <restart.h>
#include <string.h>
#include <unistd.h>

static unsigned currentworking = 0, maxworker = 0;

unsigned getMaxWorkers() {
    return maxworker;
}

unsigned increaseWorkingCount(sem_t *locker) {
    while (sem_wait(locker) == -1)
        continue;
    unsigned tmp = ++currentworking;
    if (tmp > maxworker)
        maxworker = currentworking;
    while (sem_post(locker) == -1)
        continue;
    return tmp;
}

unsigned decreaseWorkingCount(sem_t *locker) {
    while (sem_wait(locker) == -1)
        continue;
    unsigned tmp = --currentworking;
    while (sem_post(locker) == -1)
        continue;
    return tmp;
}

int doesStringFound(int openedFile, const char stringtaken[]) {
    int readCharactersFromFile = 0, sameParts = 1, error = 0;
    char charfromFile = 0;
    off_t pos = lseek(openedFile, 0, SEEK_CUR);
    while (sameParts != strlen(stringtaken) && error != 1) {
        if (r_read(openedFile, &charfromFile, sizeof(char)) > 0) {
            if (!(charfromFile == '\n' || charfromFile == '\t' || charfromFile == ' ') &&
                (tolower(charfromFile) == tolower(stringtaken[sameParts])))
                ++sameParts;
        } else
            error = 1;
        ++readCharactersFromFile;
    }
    lseek(openedFile, pos, SEEK_SET);
    if (error == 0)
        return 1;
    return 0;
}

int log_all(char index[FOUNDABLE_MAX][BUFFER_SIZE], parameters *param) {
    while (sem_wait(param->log_sem) == -1)
        fprintf(stderr, ">>> sem_wait path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
                param->referance_path, pthread_self(), getpid());

    int i = 0;
    while (pthread_rwlock_wrlock(&param->logging) != 0)
        fprintf(stderr, ">>> pthread_rwlock_wrlock path: %s%s (thread_id: %ld, pid: %d)\n",
                param->base_path, param->referance_path, pthread_self(), getpid());
    for (i = 0; i < FOUNDABLE_MAX && 0 != index[i]; ++i)
        r_write(param->logFile, &index[i], strlen(index[i]) * sizeof(char));
    while (pthread_rwlock_unlock(&param->logging) != 0)
        fprintf(stderr, ">>> pthread_rwlock_unlock path: %s%s (thread_id: %ld, pid: %d)\n",
                param->base_path, param->referance_path, pthread_self(), getpid());
    while (sem_post(param->log_sem) == -1)
        fprintf(stderr, ">>> sem_post path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
                param->referance_path, pthread_self(), getpid());
    return 0;
}

parameters *searchFile(parameters *param) {
    char path[PATH_MAX] = {0}, oneCharFromFile = 0, logging_index[FOUNDABLE_MAX][BUFFER_SIZE] = {0};
    int line_count = 1,   /*Line*/
        columninFile = 1, /*Column*/
        file_dest = FAILURE_CASE;
    param->result.line_count = 0;
    param->result.occurences = 0;
    fprintf(stderr, "> searchFile  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());
    if (NULL == param) {
        fprintf(stderr, "Param is NULL (thread_id: %ld, pid: %d)", pthread_self(), getpid());
        return param;
    }
    fprintf(stderr, "> increaseWorkingCount  path: %s%s (thread_id: %ld, pid: %d)\n",
            param->base_path, param->referance_path, pthread_self(), getpid());
    increaseWorkingCount(param->counter_sem);
    fprintf(stderr, "< increaseWorkingCount  path: %s%s (thread_id: %ld, pid: %d)\n",
            param->base_path, param->referance_path, pthread_self(), getpid());
    snprintf(path, PATH_MAX, "%s%s", param->base_path, param->referance_path);
    if (FAILURE_CASE == (file_dest = r_open2(path, O_RDONLY))) {
        fprintf(stderr, "Open %s is failed  (thread_id: %ld, pid: %d)", param->referance_path,
                pthread_self(), getpid());
        return param;
    }
    // Reads the first character from the file.
    fprintf(stderr, "> work  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());
    while (r_read(file_dest, &oneCharFromFile, sizeof(char)) > 0) {
        if (oneCharFromFile == '\n') {
            // If the character is '\n',
            // Increase the rowinLine and set columninLine to 1.
            ++param->result.line_count;
            columninFile = 0;
        } else if (tolower(oneCharFromFile) == tolower(param->query[0])) {
            // If the character is the same with the starting char of the given
            // query, Start to search for all the query.
            if (doesStringFound(file_dest, param->query) == 1) {
                ++param->result.occurences;
                snprintf(logging_index[param->result.occurences - 1], BUFFER_SIZE,
                         "%d %ld %s : [%d, %d] %s first character is found.\n", getpid(),
                         pthread_self(), path, param->result.line_count, columninFile,
                         param->query);
            }
        }
        ++columninFile;
    }

    fprintf(stderr, "< work  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());

    fprintf(stderr, "> log  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());
    if (log_all(logging_index, param) != 0)
        fprintf(stderr, "Failed to log all. (thread_id: %ld, pid: %d, file %s/%s)", pthread_self(),
                getpid(), param->base_path, param->referance_path);

    fprintf(stderr, "< log  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());
    decreaseWorkingCount(param->counter_sem);
    fprintf(stderr, "< searchFile  path: %s%s (thread_id: %ld, pid: %d)\n", param->base_path,
            param->referance_path, pthread_self(), getpid());
    return param;
}