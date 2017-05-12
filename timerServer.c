#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

static volatile sig_atomic_t doneflag = 0;


static void catched(int signo)
{
	doneflag = 1;
}


int main(int argc, char * argv[])
{
	int fifoName = 0, income = 0;
	pid_t mypid = getpid(), intrPid;
	time_t end, start;
	struct sigaction catcher;
	catcher.sa_handler = catched;
	catcher.sa_flags = 0;
	if((sigemptyset(&catcher.sa_mask) == -1) || (sigaction(SIGINT, &catcher, NULL))) 
		perror("Failed to ignor interrupt signal.");
	if(argc != 4)
	{
		return -1;
	}
	if((mkfifo(argv[3], FIFO_PERMS) == -1))
		perror("Failed to create named pipe");
	if(((fifoName = open(argv[3], O_WRONLY)) == -1))
		perror("Error on opening named pipe");
	write(fifoName, &mypid, sizeof(mypid) + 1);
	do
	{
		for()
	} while(!doneflag);
	fprintf(stderr, "ctrl-c catched\n");
	unlink(argv[3]);
	return 0;
}

//
	double Determinant(double **a,int n)
{
    int i,j,j1,j2 ;                    // general loop and matrix subscripts
    double det = 0 ;                   // init determinant
    double **m = NULL ;                // pointer to pointers to implement 2d
                                       // square array

    if (n < 1)    {   }                // error condition, should never get here

    else if (n == 1) {                 // should not get here
        det = a[0][0] ;
        }

    else if (n == 2)  {                // basic 2X2 sub-matrix determinate
                                       // definition. When n==2, this ends the
        det = a[0][0] * a[1][1] - a[1][0] * a[0][1] ;// the recursion series
        }


                                       // recursion continues, solve next sub-matrix
    else {                             // solve the next minor by building a
                                       // sub matrix
        det = 0 ;                      // initialize determinant of sub-matrix

                                           // for each column in sub-matrix
        for (j1 = 0 ; j1 < n ; j1++) {
                                           // get space for the pointer list
            m = (double **) malloc((n-1)* sizeof(double *)) ;

            for (i = 0 ; i < n-1 ; i++)
                m[i] = (double *) malloc((n-1)* sizeof(double)) ;
                       //     i[0][1][2][3]  first malloc
                       //  m -> +  +  +  +   space for 4 pointers
                       //       |  |  |  |          j  second malloc
                       //       |  |  |  +-> _ _ _ [0] pointers to
                       //       |  |  +----> _ _ _ [1] and memory for
                       //       |  +-------> _ a _ [2] 4 doubles
                       //       +----------> _ _ _ [3]
                       //
                       //                   a[1][2]
                      // build sub-matrix with minor elements excluded
            for (i = 1 ; i < n ; i++) {
                j2 = 0 ;               // start at first sum-matrix column position
                                       // loop to copy source matrix less one column
                for (j = 0 ; j < n ; j++) {
                    if (j == j1) continue ; // don't copy the minor column element

                    m[i-1][j2] = a[i][j] ;  // copy source element into new sub-matrix
                                            // i-1 because new sub-matrix is one row
                                            // (and column) smaller with excluded minors
                    j2++ ;                  // move to next sub-matrix column position
                    }
                }

            det += pow(-1.0,1.0 + j1 + 1.0) * a[0][j1] * Determinant(m,n-1) ;
                                            // sum x raised to y power
                                            // recursively get determinant of next
                                            // sub-matrix which is now one
                                            // row & column smaller

            for (i = 0 ; i < n-1 ; i++) free(m[i]) ;// free the storage allocated to
                                            // to this minor's set of pointers
            free(m) ;                       // free the storage for the original
                                            // pointer to pointer
        }
    }
    return(det) ;
}