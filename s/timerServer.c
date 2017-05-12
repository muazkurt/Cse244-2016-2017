//Libraries
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <sys/time.h>
	#include <time.h>
	#include <setjmp.h>
	#include <dirent.h>
	#include <math.h>

//Defines
	#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

	#define PID_MAX 10
	
	
//Globals
	static int pipeline[2];

	static volatile sig_atomic_t meet = 0;

	volatile sig_atomic_t checker = 0;

//signals
	//Siguser1 handler
		static void handshakeHandler(int signo, siginfo_t *sender, void *info)
		{
			if(meet == 0)
			{
				if(pipe(pipeline) == -1)
				{
					perror("Failed to create pipe");
					return;
				}
				if(write(pipeline[1], &sender->si_pid, sizeof(sender->si_pid)) != sizeof(sender->si_pid))
				{
					perror("Failed to writing pipe");
					return;
				}
				if(close(pipeline[1]) == -1)
				{
					perror("Failed to close pipe");	
					return;
				}
				meet = 1;
			}
			else 
				meet = 0;
		}

	//Sigcont handler
		static void syncHandler(int signo)
		{
			++checker;
		}

	//Sigintr handler *Main*
		static void interruptHandler(int signo, siginfo_t *sender, void *info)
			{
				meet = -1;
			}

	//Sigintr handler *Child*
		static void interruptHandlerC(int signo, siginfo_t *sender, void *info)
			{
				checker = -1;
			}
	//Sigusr2 Handler
		static void sigusr2Handler(int signo, siginfo_t *sender, void *info)
			{
				if(meet == -1);
				else checker = -1;
			}
//functions

	int **createMatrix(int **input, int size)
		{
			int j1, i;
			for (j1 = 0 ; j1 < (size); j1++)
				{
					input = (int **) malloc((size)* sizeof(int *));
					for (i = 0 ; i < (2 * size); i++)
						input[i] = (int *) malloc((size)* sizeof(int)) ;
			    }
		    return input;
		}
	double Determinant(int **a,int n)
		{
			int i, j, j1, j2;
		    double det = 0;
		    int **m = NULL ;

		    if (n < 1)
		    	det = -1;
			else if (n == 1)
				det = a[0][0];
		    else if (n == 2)
			    {
					det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
				}

		    else
		    {
		    	det = 0 ;
		        for (j1 = 0 ; j1 < n ; j1++)
			        {
			        	m = (int **) malloc((n-1)* sizeof(int *)) ;

			            for (i = 0 ; i < n-1 ; i++)
			                m[i] = (int *) malloc((n-1)* sizeof(int)) ;

			            for (i = 1 ; i < n ; i++)
				            {
				                j2 = 0 ;
				                for (j = 0 ; j < n ; j++)
				                {
				                    if (j == j1)
				                    	continue ;

				                    m[i-1][j2] = a[i][j] ;
				                    j2++ ;
				                }
				            }
			            det += pow(-1.0,1.0 + j1 + 1.0) * a[0][j1] * Determinant(m,n-1) ;

			            for (i = 0 ; i < n-1 ; i++)
			            	free(m[i]);

			            free(m) ;
			        }
		    }
		    return(det) ;
		}
	int **fillArray(int **input, int size)
		{
			double result;
			int i, j;
			while((result = Determinant(input, size)) == 0)
			{

				for(i = 0; i < (size); ++i)
				{
					for(j = 0; j < (size); ++j)
					{
						input[i][j] = rand() % 20;
					}
				}
			}
			return input;
		}
	void printPid(pid_t in, int fileno)
		{
			char bufferS[2 * PID_MAX],
					bufferC;
			snprintf(bufferS, 2 * PID_MAX, "Client pid:");
			write(fileno, &bufferS, strlen(bufferS));
			bufferC = ' ';
			write(fileno, &bufferC, 1);
			snprintf(bufferS, PID_MAX, "%ld", (long)in);
			write(fileno, &bufferS, strlen(bufferS));
			bufferC = '\n';
			write(fileno, &bufferC, 1);
		}
	void printDif(double difRes, int fileno)
		{
			char bufferS[2 * PID_MAX],
				 bufferC;
			snprintf(bufferS, 2 * PID_MAX, "Determinant:");
			write(fileno, &bufferS, strlen(bufferS));
			bufferC = ' ';
			write(fileno, &bufferC, 1);
			snprintf(bufferS, 2 * PID_MAX, "%.2f", difRes);
			write(fileno, &bufferS, strlen(bufferS));
			bufferC = '\n';
			write(fileno, &bufferC, 1);
		}
//main
	int main(int argc, char * argv[])
	{
	//defining variables
		int 
			fifoName = 0,
			arraySize = atoi(argv[2]),
			x = 0, y = 0, tmp = 0,
			logfile = 0;
		
		int **array;

		double difRes = 0;
		char 
			stringPid[PID_MAX],
			logfilename[NAME_MAX];
		
		pid_t 
			  mypid = getpid(),
			  clientPid = 0,
			  childPid = 0;

		struct sigaction 
						 hand_shake,
						 sync, 
						 interrupt, interruptC,
						 sigusr2;

		sigset_t suspendingSignals;

		union sigval value;

		struct timespec ts;	

	//Usage
		if(argc != 4)
		{
			fprintf(stderr, "Usage:\n");
			fprintf(stderr, "%s time(in m.sec) array_size fifo_name\n", argv[0]);
			return -1;
		}
	//Create array
		snprintf(logfilename, NAME_MAX, "log/timeServer.log");
		while(open(logfilename, O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
				perror("Failed to create logfile");
		array = createMatrix(array, 2 * arraySize);
    	clock_gettime(CLOCK_MONOTONIC, &ts);
	    srand((time_t)ts.tv_nsec);
	//signals
		value.sival_int = arraySize;
		//Hand shake set up sigusr1
			hand_shake.sa_sigaction = &handshakeHandler;
			hand_shake.sa_flags = SA_SIGINFO;
			if((sigemptyset(&hand_shake.sa_mask) == -1)
				|| (sigaddset(&hand_shake.sa_mask, SIGUSR1) == -1)
				|| (sigaction(SIGUSR1, &hand_shake, NULL) == -1))
				{
					perror("Failed to handle hand shake");
					return -1;
				}

		//Syncronisation set up
			sync.sa_handler = &syncHandler;
			sync.sa_flags = 0;
			if((sigemptyset(&sync.sa_mask) == -1)
				|| (sigaction(SIGCONT, &sync, NULL) == -1))
				{
					perror("Failed to handle Syncronisation");
					return -1;
				}

		//Sigint set up
			interrupt.sa_sigaction = &interruptHandler;
			interrupt.sa_flags = SA_SIGINFO;
			if((sigemptyset(&interrupt.sa_mask) == -1)
				|| (sigaction(SIGINT, &interrupt, NULL) == -1))
				{
					perror("Failed to handle interrupt");
					return -1;
				}

		//Sigusr2 set up
			sigusr2.sa_sigaction = &sigusr2Handler;
			sigusr2.sa_flags = SA_SIGINFO;
			if((sigemptyset(&sigusr2.sa_mask) == -1)
				|| (sigaction(SIGUSR2, &sigusr2, NULL) == -1))
				{
					perror("Failed to handle sigusr2");
					return -1;
				}
		
		//Sigsuspend set up
			if((sigprocmask(SIG_SETMASK, NULL, &suspendingSignals) == -1)
				|| (sigdelset(&suspendingSignals, SIGUSR1) == -1)
				|| (sigdelset(&suspendingSignals, SIGUSR2) == -1)
				|| (sigdelset(&suspendingSignals, SIGCONT) == -1)
				|| (sigdelset(&suspendingSignals, SIGINT) == -1))
				{
					perror("Failed to set up Signal suspending");
					return -1;
				}
	//Make fifo
		if(mkfifo(argv[3], FIFO_PERMS) == -1)
		{
			perror("Failed to create named pipe");
			return -1;
		}
	//loop		
		do
		{
		//Write Server pid
			if(meet == 0)
			{
				if((fifoName = open(argv[3], O_WRONLY)) == -1);
				else
				{
					if(write(fifoName, &mypid, sizeof(mypid)) != sizeof(mypid));
					else
					{	if(close(fifoName) == -1);
						else sigsuspend(&suspendingSignals);
					}
				}
			//Wait For HandShake
			}
		//If SIGUSR1 is here.
			if(meet == 1)
			{
			//Restore the handler's input.
					raise(SIGUSR1);
					logfile = open(logfilename, O_WRONLY);
				//Create Child
					if((childPid = fork()) == -1)
					{
						perror("Failed to fork");
						return -1;
					}
					//Child Job
						if(childPid == 0)
						{
							//Int handler
								interruptC.sa_sigaction = &interruptHandlerC;
								interruptC.sa_flags = SA_SIGINFO;
								if((sigemptyset(&interruptC.sa_mask) == -1)
									|| (sigaction(SIGINT, &interruptC, NULL) == -1))
									{
										perror("Failed to handle interrupt");
										_exit(0);
									}
							//Take Client pid
								if(read(pipeline[0], &clientPid, sizeof(clientPid)) != sizeof(clientPid))
								{
									perror("Failed to get client pid");
									_exit(0);
								}
								if(close(pipeline[0]) == -1)
								{
									perror("Failed to close pipe");
									_exit(0);
								}
							//Create communication fifo.
								snprintf(stringPid, PID_MAX, "%ld", (long)clientPid);
								if(mkfifo(stringPid, FIFO_PERMS) == -1)
								{
									perror("Failed to open Fifo");
									_exit(0);
								}
							//Finish hand-shake with client
								sigqueue(clientPid, SIGUSR1, value);
							//Start loop to syncronised communication
								do
								{
									//Wait array want.
										sigsuspend(&suspendingSignals);
										if(checker > tmp)
											{
												tmp = checker;
												//Take time.
												//Create matrix
													array = fillArray(array, 2 * arraySize);
												//Take time
													difRes = Determinant(array, 2 * arraySize);
												//Write to log file -> dif-time, client pid.
													printPid(clientPid, logfile);
													printDif(difRes, logfile);
												//Write array.
													if(((fifoName = open(stringPid, O_WRONLY)) == -1) && (errno != EINTR))
													{
														perror("Error on opening named pipe");
													}
													fprintf(stderr, "%ld says as\n", (long)getpid());
													for(y = 0; y < 2 * arraySize; ++y)
													{
														for(x = 0; x < 2 * arraySize; ++x)
														{
															if(write(fifoName, &array[y][x], sizeof(array[y][x])) != sizeof(array[y][x]))
																perror("Failed to write array");
														}
													}
												//TAKE YOUR ARRAY
													if(sigqueue(clientPid, SIGCONT, value) != 0)	
													{
														perror("Failed to send signal");
													}
													close(fifoName);
													//Calculate the dif.
													//Write to log dif.
											}
									//OTHER SITUATIONS
										else if(checker == -1)
										{
											if(unlink(stringPid) == -1)
												perror("Failed to unlink");
											fprintf(stderr, "ben kacar\n");
											sigqueue(clientPid, SIGINT, value);
											sigqueue(mypid, SIGINT, value);
											_exit(0);
										}
										fprintf(stderr, "dongu\n" );
								} while(checker >= 0);
							//jump child
						}
			}
			if(meet == -1)
			{
				sigqueue((-1 * getpid()), SIGUSR2, value);
			}	
		} while(meet >= 0);
	//jump main
		fprintf(stderr, "parent\n");
		if(unlink(argv[3]) == -1)
			perror("Failed to unlink");
	
		return 0;
	}
//
