#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

static volatile sig_atomic_t checkSIG2 = 0;
static volatile sig_atomic_t control = 1;
static volatile sig_atomic_t id = 0;
static volatile sig_atomic_t show = 0;
static volatile sig_atomic_t showPID = 0;
//#define FIFO_PERMS (S_IWUSR | S_IWGRP | S_IWOTH)

#define NANOSECONDS 1000000000000L
#define THOUSAND 1000L
#define MILLION 1000000L
#define BILLION 1000000000L
#define	PATH_MAX 4096

struct matrix
{
	double random[255][255];
	int size;
	pid_t serverChildPID;
};

struct clientList
{
	pid_t list[255];
	int remainingRequests;
	int indexList;
};
struct clientList cList;
pid_t childPID[255];
int j;

static void loopBreaker(int signo, siginfo_t* info, void *context)
{
	int fp;
	union sigval val;

	if(id==0)
	{
		cList.list[cList.indexList] = info->si_pid;
		cList.remainingRequests++;
		cList.indexList++;
		fprintf(stderr, "SIG1 FROM CLIENT - pid: %ld, remain: %d\n",(long)info->si_pid, cList.remainingRequests);
		if(show)
		{
			fprintf(stderr, "fifik\n" );
			mkfifo("pid_clients", 0666);
			fprintf(stderr, "ACIYOM\n" );
			fp = open("pid_clients", O_RDWR);
			fprintf(stderr, "yolluyom\n" );
			//sigqueue(showPID, SIGALRM, val);	//Yazdım.
			fprintf(stderr, "yazıcam\n" );
			write(fp, &cList.list[cList.indexList-1], sizeof(pid_t));
			fprintf(stderr, "bittim\n" );
			close(fp);
		}
	}
	else
	{
		fprintf(stderr, "sinyal-1");
		checkSIG2 = 1;
	}
}

static void giveInfo(int signo, siginfo_t* siginfo, void *context)
{
	int fp;
	int check;
	char pid[10];
	union sigval val;
	fprintf(stderr, "sinyal-2\n");
	showPID = siginfo->si_pid;

	mkfifo("pid_clients.temp", 0666);
	fp = open("pid_clients.temp", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	//sigqueue(siginfo->si_pid, SIGUSR1, val);	//Yazdım.
	check = write(fp, &cList, sizeof(struct clientList));
	fprintf(stderr, "SHOW PIDS, showpid: %ld write_check: %d\n",(long)siginfo->si_pid, check);
	
	close(fp);
	show = 1;
}


static void closeup(int signo, siginfo_t* info, void *context)
{
	int i;
	union sigval val;
	char filename[255];
	for(i = cList.indexList-1; i>=0; i--)
	{
		fprintf(stderr, "Deneme, i: %d\n", i);
		sigqueue(cList.list[i], SIGKILL, val);
		sprintf(filename,"%ld",(long)cList.list[i]);
		unlink(filename);
		sprintf(filename,"%ld.temp",(long)cList.list[i]);
		unlink(filename);
	}
	for(i = 0; i<=j-1; i++)
	{
		fprintf(stderr, "Deneme, i: %ld\n", (long)childPID[i]);
		sigqueue(childPID[i], SIGKILL, val);
	}
	if(showPID != 0)
	{
		sigqueue(showPID, SIGKILL, val);
	}

	unlink("SERVER.temp");
	unlink("pid_clients.temp");
	unlink("myfifo");
	raise(SIGKILL);
}

double determinant(double **a,int n);

int main(int argc, char* argv[])
{

	int fd, fp;
	int check;
	int i;
	int n;

	char * myfifo = "myfifo";
    char pidbuf[10];
    char export[255];
    char path[PATH_MAX];
    long timedif;
	sigset_t mask, waiting_mask, blockmask;
	FILE* logfile;
	struct sigaction act, act2, act3, terminate;
	struct matrix generatedMatrix;
	struct timespec ts; //Randomize
	struct timeval generationTimeS, generationTimeE;
    struct itimerspec nvalue;
    struct itimerspec currValue;
    struct sigevent evp;

	pid_t myPID;
	pid_t workingWith;
	
    
	union sigval val;
    
    timer_t timeid;
    
       /*CLIENT*/
    /*************/
    double **random;
	int fpClient;
	int fifo;
    int k, l;
    double det;
    char file[10];
	/************/

    cList.remainingRequests = 0;
    cList.indexList = 0;
    myPID = getpid();

    getcwd(path, PATH_MAX);
    strcat(path, "/");
    strcat(path, "LOGS");
    system("rm -rf LOGS/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);

    fprintf(stderr, "%ld\n", (long)myPID);
    i=0;
	j=0;
	n = 2;

	fp = open("SERVER.temp", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	check = write(fp, &myPID, sizeof(pid_t));
	printf("ADIMI YAZDIM: %d\n", check);
	close(fp);

/*********************************TIMER SETUP***********************************/

	evp.sigev_notify = SIGEV_NONE;

	if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &evp, &timeid) == -1) {
		perror("Failed to create a timer based on CLOCK_REALTIME");
		return -1;
	}
	
	nvalue.it_value.tv_sec = 0;
	nvalue.it_value.tv_nsec = n*MILLION;
	nvalue.it_interval.tv_sec = 0;
	nvalue.it_interval.tv_nsec = n*MILLION;

	if (nvalue.it_interval.tv_nsec >= BILLION) {
		nvalue.it_interval.tv_sec++;
		nvalue.it_interval.tv_nsec -= BILLION;
	}

	if (timer_settime(timeid, 0, &nvalue, NULL) == -1) {
		perror("Failed to set interval timer");
		return -1;
	}
/*********************************TIMER SETUP***********************************/	

/*******************************SIGNAL HANDLING*********************************/

	sigfillset(&blockmask);
	sigfillset(&mask);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGUSR2);
	sigdelset(&blockmask, SIGINT);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = loopBreaker;
	act3.sa_flags = SA_SIGINFO;
	//act3.sa_mask = mask;
	act3.sa_sigaction = giveInfo;
	terminate.sa_flags = SA_SIGINFO;
	terminate.sa_sigaction = closeup;

	if ((sigemptyset(&act.sa_mask) == -1) ||
		(sigaction(SIGRTMIN, &act, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}
	if ((sigfillset(&act3.sa_mask) == -1) ||
		(sigaction(SIGUSR2, &act3, NULL) == -1)) {
		perror("Failed to set up SIGUSR2 signal");
		return -1;
	}

	if ((sigemptyset(&terminate.sa_mask) == -1) ||
		(sigaction(SIGTERM, &terminate, NULL) == -1)) {
		perror("Failed to set up SIGUSR2 signal");
		return -1;
	}


	
	

/*******************************SIGNAL HANDLING*********************************/

    /* create the FIFO (named pipe) */
    mkfifo(myfifo, 0666);

	fd = open(myfifo, O_WRONLY);

	sprintf(pidbuf,"%ld",(long)myPID);

    //check = write(fd, pidbuf, sizeof(pidbuf));
    check = write(fd, &myPID, sizeof(myPID));

    fprintf(stderr, "::%d\n", check);

    //close(fd);

    //unlink(myfifo);


    while(control)
    {
    	sigprocmask(SIG_BLOCK, &mask, NULL); //BLOCK
		while(cList.remainingRequests == 0)
		{
			timer_gettime(timeid, &currValue);	//CHECK THE TIMER
			
			if(currValue.it_value.tv_nsec == 0){	//IF TIMER EXPIRED
				sigpending(&waiting_mask);
				if (sigismember(&waiting_mask, SIGRTMIN) || sigismember(&waiting_mask, SIGTERM)){
					fprintf(stderr, "SIGNAL RECEIVED BY CLIENT\n");
					break;
				}
			}
			//fprintf(stderr, "Uyandım\n");
		}
		sigprocmask(SIG_UNBLOCK, &mask, NULL);



		fprintf(stderr, "Ciktim- remain: %d\n", cList.remainingRequests);
		//check = write(fd, &myPID, sizeof(myPID));
//fprintf(stderr, "Yazdim\n");
		//sigprocmask(SIG_BLOCK, &blockmask, NULL); //BLOCK
		while(cList.remainingRequests != 0){
			fprintf(stderr, "girdim- remain: %d\n", cList.remainingRequests);

			childPID[j] = fork();
			workingWith = cList.list[cList.indexList - cList.remainingRequests];
			fprintf(stderr, "index - Reguest: %d - %d = %ld\n", cList.indexList, cList.remainingRequests, (long)workingWith);
			cList.remainingRequests--;
		
			if(childPID[j] == 0)
			{
			    	id = 1;
				    random = malloc((2*n)*sizeof(double *));
			        for (i=0;i<2*n;i++){
			           random[i] = malloc((2*n)*sizeof(double));
			        }
				    //sigprocmask(SIG_UNBLOCK, &blockmask, NULL); //UNBLOCK
			    
			       fprintf(stderr, "pid-file: .%ld.\n", (long)workingWith);

			       sprintf(file,"%ld", (long)workingWith);

				    if(mkfifo(file, 0666) == -1){
						perror("Failed to FIFO");
						exit(1);
					}

					fifo = open(file, O_RDWR);
					fprintf(stderr, "fifo-check: %d\n", errno);


				    generatedMatrix.size = 2*n;
				    generatedMatrix.serverChildPID = getpid();

				    checkSIG2 = 1;
				    fprintf(stderr, "checkSIG2: %d\n", checkSIG2);
					while(control){
						if(checkSIG2){
							sigprocmask(SIG_BLOCK, &mask, NULL);

							det = 0;
						    while(det == 0){

						    	clock_gettime(CLOCK_MONOTONIC, &ts);
							    srand((time_t)ts.tv_nsec);

							    gettimeofday(&generationTimeS, NULL);
							    for(k = 0; k<(2*n); k++){
							        for(l = 0; l<2*n; l++){
							            random[k][l] = rand() % 20;
							        }
							    }
							    gettimeofday(&generationTimeE, NULL);

							    timedif = ((MILLION*(generationTimeE.tv_sec - generationTimeS.tv_sec) +
											(generationTimeE.tv_usec - generationTimeS.tv_usec)))/1000;
							    //fprintf(stderr, "secE: %d secS: %d usecE: %d usecS: %d\n", (int)generationTimeE.tv_sec, (int)generationTimeS.tv_sec
							    //, (int)generationTimeE.tv_usec , (int)generationTimeS.tv_usec );
							    det = determinant(random, 2*n);
							    //fprintf(stderr, "det: %.1lf\n", det);
							}

							sleep(3);
							for(k = 0; k<2*n; k++){
						        for(l = 0; l<2*n; l++){
						        	fprintf(stderr,"%.1lf\t", random[k][l]);
						        }

						        fprintf(stderr, "\n\n");
						    }

						    for(k = 0; k<2*n; k++){
						        for(l = 0; l<2*n; l++){
						        	generatedMatrix.random[k][l] = random[k][l];
						        }
						    }

						    sigprocmask(SIG_BLOCK, &blockmask, NULL);

							logfile = fopen("LOGS/TimerServer.log", "a+");

							fprintf(stderr, "LOGLUYORUM\n");

							sprintf(export, "Generation Time: %ldms, PID: %ld, Determinant: %.1lf", timedif, (long)workingWith
							, det); /*Logging*/
							fprintf(logfile, "%s\n", export);
							fprintf(stderr, "%s\n", export);

							fprintf(stderr, "LOGLADIM\n");
							
							fclose(logfile);
							sigprocmask(SIG_UNBLOCK, &blockmask, NULL); //UNBLOCK

							
							sigqueue(workingWith, SIGUSR1, val);
							check = write(fifo, &generatedMatrix, sizeof(struct matrix));
							fprintf(stderr, "check-fifo-write: %d\n", check);
							
						}
					    //close(fifo);
					    checkSIG2 = 0;
					    sigprocmask(SIG_UNBLOCK, &mask, NULL);
					    //fprintf(stderr, ".");
					    //sleep(1);
						//exit(1);
					}

			}
			j++;
		sigprocmask(SIG_UNBLOCK, &blockmask, NULL); //UNBLOCK
		}
	}

}

double determinant(double **a,int n)
{
   int i,j,j1,j2;
   double det = 0;
   double **m = NULL;

   if (n < 1) { 

   } else if (n == 1) { 
      det = a[0][0];
   } else if (n == 2) {
      det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
   } else {
      det = 0;
      for (j1=0;j1<n;j1++) {
      
         m = malloc((n-1)*sizeof(double *));
         for (i=0;i<n-1;i++)
            m[i] = malloc((n-1)*sizeof(double));
         for (i=1;i<n;i++) {
            j2 = 0;
            for (j=0;j<n;j++) {
               if (j == j1)
                  continue;
               m[i-1][j2] = a[i][j];
               j2++;
            }
         }
         det += pow(-1.0,1.0+j1+1.0) * a[0][j1] * determinant(m,n-1);
         for (i=0;i<n-1;i++)
         {
         	free(m[i]);
         }          
         free(m);
      }
   }
   return(det);
}
