#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define PATH_SIZE 15

static volatile sig_atomic_t control = 1;
static volatile sig_atomic_t server = 1;
//#define FIFO_PERMS (S_IWUSR | S_IWGRP | S_IWOTH)


struct results
{
	double result1;
	double result2;
	long timeElapsed1;
	long timeElapsed2;
	pid_t myPID;
};

struct clientList
{
	pid_t list[255];
	int remainingRequests;
	int indexList;
};

struct readList
{
	pid_t list[255];
	sig_atomic_t remainingRequests;
	int indexList;
};
struct readList rList;

struct newClient
{
	pid_t list[255];
	int remainingRequests;
	int indexList;
};
struct newClient newList;

pid_t serverP;

static void serverHandler(int signo, siginfo_t* info, void *context)
{
	int fp;

	//fprintf(stderr, "Serverdan pidleri okumam icin sinyal geldi\n");
	fp = open("pid_clients", O_RDONLY);
	read(fp, &newList.list[newList.indexList], sizeof(pid_t));
	//fprintf(stderr, "signal from server: YENI CLIENT VAR : %ld\n", (long)newList.list[newList.indexList]);
	newList.indexList++;
	newList.remainingRequests++;
	close(fp);
	server = 0;
}

static void clientHandler(int signo, siginfo_t* info, void *context)
{
	rList.list[rList.indexList] = info->si_pid;
	fprintf(stderr, "::SINYAL::%ld\n", (long)info->si_pid);
	rList.remainingRequests++;
	//rList.indexList++;
}


static void closeup(int signo, siginfo_t* info, void *context)
{
	union sigval val;
	sigqueue(serverP, SIGTERM, val);
}



int main(int argc, char* argv[])
{

	int fd, fp, fp2, i, j;
	int check;
	char* filename;
	char export[255];
	char print[255];
	char ch;
	pid_t serverPID, childPID, workingWith, childPID2;
	union sigval val;
	struct sigaction act, act2 ,terminate;
	struct results info;
	struct clientList cList;
	sigset_t waitServer, waitClient, mask, newLinkage;
	FILE* logfile;

	cList.remainingRequests = 0;
    cList.indexList = 0;
    rList.remainingRequests = 0;
    rList.indexList = 0;
    newList.remainingRequests = 0;
    newList.indexList = 0;

	fp = open("SERVER.temp", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	check = read(fp, &serverPID, sizeof(pid_t));
	//fprintf(stderr, "read serverPID check: %d, pid: %ld\n", check, (long)serverPID);
	close(fp);

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = serverHandler;
	act2.sa_flags = SA_SIGINFO;
	act2.sa_sigaction = clientHandler;
	terminate.sa_flags = SA_SIGINFO;
	terminate.sa_sigaction = closeup;

	sigfillset(&waitServer);
	sigfillset(&mask);
	sigfillset(&waitClient);
	sigfillset(&newLinkage);
	sigdelset(&waitServer, SIGUSR1);
	sigdelset(&waitClient, SIGUSR2);
	sigdelset(&newLinkage, SIGALRM);
	sigdelset(&waitServer, SIGINT);
	sigdelset(&waitClient, SIGINT);
	sigdelset(&mask, SIGINT);
	sigdelset(&newLinkage, SIGINT);

	if ((sigemptyset(&act.sa_mask) == -1) ||
		(sigaction(SIGALRM, &act, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}

	if ((sigfillset(&terminate.sa_mask) == -1) ||
		(sigaction(SIGINT, &terminate, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}

	if ((sigemptyset(&act2.sa_mask) == -1) ||
		(sigaction(SIGUSR2, &act2, NULL) == -1)) {
		perror("Failed to set up SIGUSR2 signal");
		return -1;
	}
	server = serverPID;
	sigqueue(serverPID, SIGUSR2, val);	//Yazdım.
	fprintf(stderr, "SENT SIGNAL2 TO SERVER\n");

	if(server)
	{
		/****************READING FROM SERVER*******************/

		fp2 = open("pid_clients.temp", O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
		while(check = read(fp2, &cList, sizeof(struct clientList)) == -1);
		//fprintf(stderr, "%ld - READ PIDS, read_check: %d, server stat: %d\n", (long)getpid(),check, server);
		close(fp2);

		/****************READING FROM SERVER*******************/

		childPID = fork();
		if(childPID == 0)
		{

			//fprintf(stderr, "list: %d\n",cList.indexList);
			/*****************SIGNALING CLIENTS********************/
			for(i = 0; i <= cList.indexList - 1; i++)
			{
				sigqueue(cList.list[i], SIGUSR2, val);	//Yazdım.
				fprintf(stderr, "%ld - SIGNALED THE CLIENTS: %ld\n",(long)getpid(),(long)cList.list[i]);
			}
			/*****************SIGNALING CLIENTS********************/

			while(control)
			{
				/*****************WAITING CLIENTS********************/
				sigprocmask(SIG_BLOCK, &mask, NULL);
				if(rList.remainingRequests == 0){
					//fprintf(stderr, "server stat: %d\n", server);
					fprintf(stderr, "WAITING THE CLIENTS\n");
					sigsuspend(&waitClient);	
					fprintf(stderr, "CLIENTS - UYANDIM\n");
				}
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
				/*****************WAITING CLIENTS********************/
							fprintf(stderr, "ara kisma geldim\n");	
							//sleep(1);

				/*****************READING CLIENTS********************/
				sigprocmask(SIG_BLOCK, &mask, NULL);
				while(rList.remainingRequests != 0)
				{
					fprintf(stderr,"Girdim, REMAIN: %d\n", rList.remainingRequests);
					filename = malloc(PATH_SIZE*sizeof(char));

					workingWith = rList.list[rList.indexList];
					fprintf(stderr, "index - Reguest: %d - %d = %ld\n", rList.indexList, rList.remainingRequests, (long)workingWith);
					rList.remainingRequests--;
					sigprocmask(SIG_UNBLOCK, &mask, NULL);
					sprintf(filename,"%ld.temp",(long)workingWith);
					fprintf(stderr, "acilacak dosya: %s\n", filename);	
					fprintf(stderr,"Açacam\n");
					fd = open(filename, O_RDWR);
								fprintf(stderr,"OKicam\n");
					check = read(fd, &info, sizeof(struct results));
					fprintf(stderr,"Okudum\n");
					fprintf(stderr, "check-read: %d\n", check);
					//sleep(1);
					//unlink(filename);
					free(filename);
					close(fd);
				/*****************READING CLIENTS********************/
					//fprintf(stderr, "Okuyacagimi okudum, logluyorum\n" );
					/******************************LOGGING OPERATION*******************************/
					logfile = fopen("LOGS/ShowResult.log", "a+");
					sigprocmask(SIG_BLOCK, &mask, NULL);
					i=1;
					j=0;
					while(ch!=EOF)
					{
						ch = fgetc(logfile);
						if(ch == '\n')
						{
							j++;
							fprintf(stderr,"i: %d, 	j: %d\n", i, j);
							if(j % 3 == 0)
							{
								i++;
							}
						}
					}
					ch = 'a';
					sigprocmask(SIG_UNBLOCK, &mask, NULL);
					rewind(logfile);

					sprintf(export,"Matrix %d, %ld\n%lf,\t%ld\n%lf,\t%ld\n", i, (long)info.myPID
						, info.result1, info.timeElapsed1, info.result2, info.timeElapsed2);
					
					fprintf(stderr,"%ld \t %.0lf \t %.0lf\n", (long)info.myPID, info.result1, info.result2);

					////fprintf(stderr, "%s\n", print); /*Printing*/
					fprintf(stderr,"export edecem\n");
					fprintf(logfile, "%s", export); /*Logging*/
					fprintf(stderr,"export ettim\n");
					
					fclose(logfile);
					/******************************LOGGING OPERATION*******************************/
					
				}
				
			 }
		}
		while(control){

			sigprocmask(SIG_BLOCK, &mask, NULL);
			if(newList.remainingRequests == 0){
				//fprintf(stderr, "server stat: %d\n", server);
				fprintf(stderr, "NEW - WAITING THE SERVER\n");
				sigsuspend(&newLinkage);	
				fprintf(stderr, "NEW - SERVER - UYANDIM\n");
			}
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			
			childPID2 = fork();
			if(childPID2 == 0){
				/*****************SIGNALING CLIENTS********************/
				sigqueue(newList.list[newList.indexList-1], SIGUSR2, val);	//Yazdım.
				//fprintf(stderr, "%ld - SIGNALED THE CLIENTS: %ld\n",(long)getpid(),(long)cList.list[i]);
				/*****************SIGNALING CLIENTS********************/
				while(control){
					/*****************WAITING NEW CLIENTS********************/
					
					sigprocmask(SIG_BLOCK, &mask, NULL);
					if(newList.remainingRequests == 0){
						fprintf(stderr, "NEW - WAITING NEW CLIENT\n" );
						sigsuspend(&waitClient);	
						fprintf(stderr, "NEW - WAKE UP - NEW CLIENT\n" );
					}
					sigprocmask(SIG_UNBLOCK, &mask, NULL);
					/*****************READING NEW CLIENTS********************/
					
					filename = malloc(PATH_SIZE*sizeof(char));

					workingWith = newList.list[newList.indexList - 1];
			//fprintf(stderr, "SERVER PART - working with: %ld\n", (long)workingWith);
					sprintf(filename,"%ld.temp",(long)workingWith);
				//fprintf(stderr, ":::::::SERVER PART - %s\n", filename);
					fd = open(filename, O_RDONLY);
									
					read(fd, &info, sizeof(struct results));

					//unlink(filename);
					free(filename);
					close(fd);

					/******************************LOGGING OPERATION*******************************/
					logfile = fopen("LOGS/ShowResult.log", "a+");
					sigprocmask(SIG_BLOCK, &mask, NULL);
					i=1;
					j=0;
					while(ch!=EOF)
					{
						ch = fgetc(logfile);
						if(ch == '\n')
						{
							j++;
							fprintf(stderr,"i: %d, 	j: %d\n", i, j);
							if(j % 3 == 0)
							{
								i++;
							}
						}
					}
					ch = 'a';
					sigprocmask(SIG_UNBLOCK, &mask, NULL);
					rewind(logfile);

					sprintf(export,"Matrix %d, %ld\n%.0lf,\t%ld\n%.0lf,\t%ld\n", i, (long)info.myPID
						, info.result1, info.timeElapsed1, info.result2, info.timeElapsed2);
						
					//fprintf(stderr,"SERVER PART - %ld \t %.0lf \t %.0lf\n", (long)info.myPID, info.result1, info.result2);
					//sleep(1);
					////fprintf(stderr, "%s\n", print); /*Printing*/

					fprintf(logfile, "%s", export); /*Logging*/
					
					fclose(logfile);
					newList.remainingRequests--;
					/******************************LOGGING OPERATION*******************************/

					
				}
			}
			newList.remainingRequests--;
		}
	}
		
}

