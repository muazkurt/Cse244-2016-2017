//Libraries
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
	#include <math.h>
	#include <signal.h>
	#include <setjmp.h>

//defines
	#define PID_MAX 10
//Globals
	static volatile sig_atomic_t syncCount = 0;
	static volatile sig_atomic_t size_array = 0;
	static sigjmp_buf 
						jumpbufINT,
						jumpbufUSR2;
	pid_t connect = 0;

//Signal
	static void syncHandler(int signo)
		{
			if(syncCount == 0)
				syncCount = 1;
			else
				syncCount = 0;
		}
	
	static void meetChild(int signo, siginfo_t *sender, void *info)
		{
			connect = sender->si_pid;
			size_array = sender->si_value.sival_int;
		}

	static void intrupt(int signo)
		{
			siglongjmp(jumpbufINT, 1);
		}

	static void intrupt2(int signo)
		{
			siglongjmp(jumpbufUSR2, 1);
		}
		

//Functions
	//Take Server Pid
		pid_t getserverPid(char *fifoname)
			{
				int namedPipe = 0;
				pid_t serverPid = 0;
				if((namedPipe = open(fifoname, O_RDONLY)) == -1)
					perror("Failed to open named pipe");
				if(read(namedPipe, &serverPid, sizeof(serverPid)) != sizeof(serverPid))
					perror("Failed to read named pipe");
				if(close(namedPipe) == -1)
					perror("Failed to close named pipe");
				return serverPid;
			}
	//Create n sized 2d matrix
		int **createMatrix(int **input, int size)
			{
				int j1, i;
				for (j1 = 0 ; j1 < (size); j1++)
					{
						input = (int **) malloc((size)* sizeof(int *));
						for (i = 0 ; i < (size); i++)
							input[i] = (int *) malloc((size)* sizeof(int)) ;
				    }
			    return input;
			}
	//Determinant of n sized matrix
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
			    	m = (int **) malloc((n-1)* sizeof(int *)) ;
			    	for (i = 0 ; i < n-1 ; i++)
			    		m[i] = (int *) malloc((n-1)* sizeof(int)) ;
			    	for (j1 = 0 ; j1 < n ; j1++)
				        {

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
				        }
		            for (i = 0 ; i < n-1 ; i++)
		            	free(m[i]);

		            free(m) ;
			    }
			    return(det) ;
			}
	//Transpose of the matrix
		void Transpose(int **a,int n)
			{
				int i, j;
				int tmp;
				for(i = 1; i < n; i++)
				{
					for(j = 0; j < i; j++)
					{
						tmp = a[i][j];
						a[i][j] = a[j][i];
						a[j][i] = tmp;
					}
				}
			}
	//ShiftedInverse of n sized square matrix
		void ShiftedInverse(int **a,int n,int **b)
			{
				int i, j, ii, jj, i1, j1;
				double det;
				for (j = 0; j < n; j++)
				{
					for (i = 0; i < n; i++)
					{
						i1 = 0;
				        for (ii = 0; ii < n; ii++)
				        {
				            if (ii == i)
				               continue;
				            j1 = 0;
				            for (jj = 0; jj < n; jj++)
				            {
				            	if (jj == j)
				            		continue;
				            	b[i1][j1] = a[ii][jj];
				            	j1++;
				            }
				            i1++;
				        }
				        det = Determinant(b, n - 1);
				        b[i][j] = pow(-1.0, i + j + 2.0) * det;
				    }
				}
				Transpose(b, n);
			}
	//Prints as matlab format
		void printLog(int **workedarray, int size, int logfile)
			{
				int x, y;
				char 
						bufferStr[10],
						bufferChar;
				bufferChar = '\n';
				write(logfile, &bufferChar, 1);
				for(y = 0; y < 2 * size; ++y)
					{
						for(x = 0; x < 2 * size; ++x)
						{
							snprintf(bufferStr, 10, "\t%d", workedarray[y][x]);
							write(logfile, &bufferStr, strlen(bufferStr));
						}
						bufferChar = '\n';
						write(logfile, &bufferChar, 1);
				}
			}
	//Prints error
		void printError(int fileno)
			{
				char buffer[NAME_MAX];
				snprintf(buffer, NAME_MAX, "INTRUPTED");
				write(fileno, &buffer, strlen(buffer));
			}
//main
	int main(int argc, char *argv[])
	{
	//defining variables
		int 
			excist = 0,
			namedPipeC = 0,
			pipeline[2], 
			innerpipeline[2],
			logfile = 0,
			x = 0, y = 0, tmp = 0, itmp = 0,
			**myarray,
			**workedarray,
			**deepbuffer,
			**deepbuffer2,
			waitSignal = 1;

		double result = 0.0;

		char 
			stringly[PID_MAX],
			logfilename[NAME_MAX];

		pid_t 
			  serverPid = 0, 
			  mypid = getpid(),
			  childPid = 0,
			  InChildPid = 0;
		
		struct sigaction 
						 hand_shake,
						 sync, 
						 intruptA,
						 intrupt2A;
		
		sigset_t suspendingSignals;

		union sigval value;	
	//usage	
		if(argc != 2)
		{
			fprintf(stderr, "Usage:\n");
			fprintf(stderr, "%s fifo_name\n", argv[0]);
			return -1;
		}
	//signals start
		//HAND-SHAKING signal handling
			hand_shake.sa_sigaction = &meetChild;
			hand_shake.sa_flags = SA_SIGINFO;
			if((sigemptyset(&hand_shake.sa_mask) == -1)
				|| (sigaddset(&hand_shake.sa_mask, SIGUSR1) == -1)
				|| (sigaction(SIGUSR1, &hand_shake, NULL) == -1)) 
				perror("Failed to create hand shake");
		
		//Syncronising by SIGCONT
			sync.sa_handler = &syncHandler;
			sync.sa_flags = 0;
			if((sigemptyset(&sync.sa_mask) == -1)
				|| (sigaction(SIGCONT, &sync, NULL) == -1))
				perror("Failed to handle Syncronisation");

		//Interrupt by SIGINT
			intruptA.sa_handler = &intrupt;
			intruptA.sa_flags = 0;
			if((sigemptyset(&intruptA.sa_mask) == -1)
				|| (sigaction(SIGINT, &intruptA, NULL) == -1))
				perror("Failed to handle Syncronisation");

			intrupt2A.sa_handler = &intrupt2;
			intrupt2A.sa_flags = 0;
			if((sigemptyset(&intrupt2A.sa_mask) == -1)
				|| (sigaction(SIGUSR2, &intrupt2A, NULL) == -1))
				perror("Failed to handle Syncronisation");

		//Sigsuspend set up
			if((sigprocmask(SIG_SETMASK, NULL, &suspendingSignals) == -1)
				|| (sigdelset(&suspendingSignals, SIGUSR1) == -1)
				|| (sigdelset(&suspendingSignals, SIGUSR2) == -1)
				|| (sigdelset(&suspendingSignals, SIGCONT) == -1)
				|| (sigdelset(&suspendingSignals, SIGINT) == -1))
					perror("Failed to set up Signal suspending");
	//GET SERVER PID
		serverPid = getserverPid(argv[1]);
		snprintf(stringly, PID_MAX, "%ld", (long)mypid);
	//SEND&WAIT SIGNAL
		snprintf(logfilename, NAME_MAX, "log/seeWhat%d.log", ++excist);
		while(open(logfilename, O_CREAT | O_EXCL, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
			{
				if(errno == EEXIST)
					snprintf(logfilename, NAME_MAX, "log/seeWhat%d.log", ++excist);
				else
				{
					perror("Failed to create logfile");
					break;
				}
			}
		if((logfile = open(logfilename, O_WRONLY)) == -1)
			perror("Failed to open log file");
		if((sigqueue(serverPid, SIGUSR1, value) == -1) && errno == ESRCH)
			perror("There is no such pid");
		//WAIT SIGUSR 1
			sigsuspend(&suspendingSignals);
			myarray = createMatrix(myarray, 2 * size_array);
			workedarray = createMatrix(workedarray, 2 * size_array);
			deepbuffer = createMatrix(deepbuffer, size_array);
			deepbuffer2 = createMatrix(deepbuffer2, size_array);
		//END OF HANDSHAKE
	//Loop

		while(waitSignal)
			{
				fprintf(stderr, "sending to %ld SIGCONT\n", (long)connect);
				if((namedPipeC = open(stringly, O_RDONLY | O_NONBLOCK)) == -1)
					perror("Failed to open named pipe");
				if(sigsetjmp(jumpbufINT, 1))
				{
					if(mypid == getpid())
					{
						sigqueue(serverPid, SIGINT, value);
					}
					else
						sigqueue(mypid, SIGINT, value);
				}
				if(sigsetjmp(jumpbufUSR2, 1))
				{
					if(mypid == getpid())
					{
						printError(logfile);
						return -1;
					}
					else
						_exit(0);
				}
			//I WANT AN ARRAY
				if(sigqueue(connect, SIGCONT, value) != 0)
					perror("Failed to send signal");
			//WAIT FOR SIGCONT
				sigsuspend(&suspendingSignals);
				fprintf(stderr, "sending to %ld SIGCONT\n", (long)connect);
				if(syncCount == 1)
					{
					//TAKING ARRAY
						for(y = 0; y < 2 * size_array; ++y)
							{
								for(x = 0; x < 2 * size_array; ++x)
								{
									if(read(namedPipeC, &myarray[y][x], sizeof(myarray[y][x])) != sizeof(myarray[y][x]))
										perror("Failed to write array");
								}
							}
						close(namedPipeC);
						result = Determinant(myarray, 2 * size_array);
						write(logfile, "Original matrix:", strlen("Original matrix:"));
						printLog(myarray, size_array, logfile);
						fprintf(stderr, "%f\n", result);
					//Array works.
						if((childPid = fork()) == -1)
							perror("Failed to fork");
						//Shifted Inverse part of function.
							if(childPid == 0)
								{
									write(logfile, "Shifted Inverse:", strlen("Shifted Inverse:"));
									pipe(pipeline);
									if((InChildPid = fork()) == -1)
										perror("Failed to fochildPidrk");
									if(InChildPid == 0) 
										{
											if(pipe(innerpipeline) == -1)
												perror("Failed to pipe");
											if((InChildPid = fork()) == -1)
												perror("Failed to fork");
											if(InChildPid == 0) 
												{
													ShiftedInverse(myarray, y, workedarray);
													for(y = 0; y < size_array; ++y)
														for(x = 0; x < size_array; ++x)
															if(write(innerpipeline[1], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
																perror("Failed");
													close(innerpipeline[1]);
													_exit(0);
												}
											for(y = 0; y < size_array; ++y)
												for(x = size_array, tmp = 0; x < 2 * size_array; ++x, ++tmp)
													deepbuffer[y][tmp] = myarray[y][x];
											tmp = 0; itmp = 0;
											ShiftedInverse(deepbuffer, size_array, deepbuffer2);
											for(y = 0; y < size_array; ++y)
												for(x = size_array, tmp = 0; x < 2 * size_array; ++x, ++tmp)
													workedarray[y][x] = deepbuffer2[y][tmp++];
											//waitchild
											while(wait(NULL) != InChildPid);
											for(y = 0; y < size_array; ++y)
												for(x = 0; x < size_array; ++x)
													if(read(innerpipeline[0], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
														perror("Failed");
											close(innerpipeline[0]);
											for(y = 0; y < size_array; ++y)
												for(x = 0; x < 2 * size_array; ++x)
													if(write(pipeline[1], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
														perror("Failed");
											close(pipeline[1]);
											_exit(0);
										}
									if(pipe(innerpipeline) == -1)
										perror("Failed to pipe");
									if((childPid = fork()) == -1)
										perror("Failed to fork");
									if(childPid == 0) 
										{
											for(y = size_array, itmp = 0; y < 2 * size_array; ++y, ++itmp)
												for(x = 0, tmp = 0; x < size_array; ++x, ++tmp)
													deepbuffer[itmp][tmp] = myarray[y][x];
											ShiftedInverse(deepbuffer, size_array, deepbuffer2);
											for(y = size_array, itmp = 0; y < 2 * size_array; ++y, ++itmp)
												for(x = 0, tmp = 0; x < size_array; ++x, ++tmp)
													workedarray[y][x] = deepbuffer2[itmp][tmp];
											for(y = size_array; y < 2 * size_array; ++y)
												for(x = 0; x < size_array; ++x)
													if(write(innerpipeline[1], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
														perror("Failed");
											close(innerpipeline[1]);
											_exit(0);
										}
									for(y = size_array, itmp = 0; y < 2 * size_array; ++y, ++itmp)
										for(x = size_array, tmp = 0; x < 2 * size_array; ++x, ++tmp)
											deepbuffer[itmp][tmp] = myarray[y][x];
									ShiftedInverse(deepbuffer, size_array, deepbuffer2);
									for(y = size_array, itmp = 0; y < 2 * size_array; ++y, ++itmp)
										for(x = size_array, tmp = 0; x < 2 * size_array; ++x, ++tmp)
											workedarray[y][x] = deepbuffer2[itmp][tmp];
									while(wait(NULL) != childPid);
									for(y = size_array; y < 2 * size_array; ++y)
											for(x = 0; x < size_array; ++x)
												if(read(innerpipeline[0], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
													perror("Failed");
									close(innerpipeline[0]);
									while(wait(NULL) != InChildPid);
									for(y = 0; y <  size_array; ++y)
										for(x = 0; x < 2 * size_array; ++x)
											if(read(pipeline[0], &workedarray[y][x], sizeof(workedarray[y][x])) != sizeof(workedarray[y][x]))
												perror("Failed");
									close(pipeline[0]);
									printLog(workedarray, size_array, logfile);
									result -= Determinant(workedarray, 2 * size_array);
									_exit(0);
								}
							while(wait(NULL) != childPid);
						//2d convolution part

					}
			}
		return 0;
	}
//