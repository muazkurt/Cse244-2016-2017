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
#include <math.h>

#define MAX_BUF 10
#define MILLION 1000000L

double shiftedInverse2D(double arr[255][255], int size);
double determinant(double **a,int n);
void cofactor(double **num, int f);
void convolution_2D(double **in, int size);
double convolution(double arr[255][255], int size);
pid_t server;
struct matrix
{
	double random[255][255];
	int size;
	pid_t serverChildPID;
};

struct results
{
	double result1;
	double result2;
	long timeElapsed1;
	long timeElapsed2;
	pid_t myPID;
};

static volatile sig_atomic_t count = 0;
static volatile sig_atomic_t received = 0;
static volatile pid_t showPID = 0;

void handler(int signo, siginfo_t* info, void *context)
{
	count++;
	write(STDERR_FILENO, "SIGUSR1 RECEIVED.", sizeof("SIGUSR1 RECEIVED."));
}

static void getShowINFO(int signo, siginfo_t* info, void *context)
{
	received = 1;
	showPID = info->si_pid;
	fprintf(stderr, "CLIENT'TAN SINYAL ALDIM\n" );
}


static void closeup(int signo, siginfo_t* info, void *context)
{
	union sigval val;
	sigqueue(server, SIGTERM, val);
}



int main()
{
    int fd, fd2, fd3;
    int check;
    int k, l, i ,j;
    int doneflag;
    int pipeline[2];
    long timedif1, timedif2;
    char * myfifo = "myfifo";
    char specialFIFO[10];
    char showResFIFO[15];
    char buf[MAX_BUF];
    double det1, det2, det;
    double **tempMatrix;
    pid_t serverPID;
    pid_t childPID;
    pid_t childPID2;
    pid_t temp;
    struct matrix matrixINFO;
    struct results sendToShow;
    struct timeval start, end;

    sigset_t mask, blockMask;
    union sigval val;
    struct sigaction act;
    struct sigaction act2;
    struct sigaction terminate;
    int flag;

    doneflag = 1;
    flag=0;
    i=0;

    act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = handler;
	terminate.sa_flags = SA_SIGINFO;
	terminate.sa_sigaction = closeup;

	if ((sigemptyset(&act.sa_mask) == -1) ||
		(sigaction(SIGUSR1, &act, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}

	if ((sigfillset(&terminate.sa_mask) == -1) ||
		(sigaction(SIGINT, &terminate, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}

	act2.sa_flags = SA_SIGINFO;
	act2.sa_sigaction = getShowINFO;

	if ((sigemptyset(&act2.sa_mask) == -1) ||
		(sigaction(SIGUSR2, &act2, NULL) == -1)) {
		perror("Failed to set up SIGUSR1 signal");
		return -1;
	}



    sigemptyset(&mask);
    sigfillset(&blockMask);
    //sigdelset(&mask, SIGUSR1);
    //sigdelset(&mask, SIGINT);

    /* open, read, and display the message from the FIFO */
    fd = open(myfifo, O_RDWR);

    check = read(fd, &serverPID, sizeof(serverPID));
    //fprintf(stderr, "check-read: %d\n", check);
    check = write(fd, &serverPID, sizeof(serverPID));
    //fprintf(stderr, "check-write: %d\n", check);
    close(fd);
    server = serverPID;
    sprintf(specialFIFO, "%ld",(long)getpid());
fprintf(stderr, "BEN: %ld\n", (long)getpid());
fprintf(stderr,"Received: %ld.\n", (long)serverPID);

check = sigqueue(serverPID, SIGRTMIN, val);
		fprintf(stderr, "%d\n", check);

	while(1){
		if(flag)
		{
			while(check = sigqueue(temp, SIGRTMIN, val) == -1);
			fprintf(stderr, "%ld - Sinyal Yolladım: %d.out\n", (long)getpid(),check);
		}
		//sleep(1);
		sigprocmask(SIG_BLOCK,&blockMask, NULL);
		while(count == 0){			
			fprintf(stderr, "%ld - bekliyorum\n", (long)getpid());
			sleep(1);
			sigpending(&mask);
			if (sigismember(&mask, SIGUSR1)){
				fprintf(stderr, "%ld - SIGNAL RECEIVED BY SERVER\n", (long)getpid());
				break;
			}	
		}
		sigprocmask(SIG_UNBLOCK, &blockMask, NULL);
		count--;

		//fprintf(stderr, "Fifo Dosya Ac PID: %ld\n", (long)getpid() );
		fd2 = open(specialFIFO, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
		//fprintf(stderr, "Fifo Dosya Oku\n" );
		if(check = read(fd2, &matrixINFO, sizeof(struct matrix)) == -1)
		{
			while(check = sigqueue(temp, SIGRTMIN, val) == -1);
		}
		close(fd2);
		//fprintf(stderr, "Ikisini de hallettim.\n" );
		//fprintf(stderr, "read-check-fifo: %d\n", check);
		
		//fprintf(stderr, "size: %d\tpid: %ld\n", matrixINFO.size, (long)matrixINFO.serverChildPID);

		if(!flag){
			temp = matrixINFO.serverChildPID;
			while(check = sigqueue(temp, SIGRTMIN, val) == -1);
			fprintf(stderr, "%ld - Sinyal Yolladım: %d.in\n", (long)getpid(),check);
		}
		flag = 1;
		for(k = 0; k<4; k++){
	        for(l = 0; l<4; l++){
	        	//fprintf(stderr,"%.1lf\t", matrixINFO.random[k][l]);
	        }
	        //fprintf(stderr, "\n\n");
	    }
	    fprintf(stderr, "MATRIX\n");

	    

	    pipe(pipeline);
		childPID = fork();


		if(childPID == 0)
		{

			tempMatrix = malloc((matrixINFO.size)*sizeof(double *));
			for (i=0;i<matrixINFO.size;i++)
			   tempMatrix[i] = malloc((matrixINFO.size)*sizeof(double));

			for(k = 0; k<matrixINFO.size; k++){
		        for(l = 0; l<matrixINFO.size; l++){
		        	tempMatrix[k][l] = matrixINFO.random[k][l];
		        }
		    }
		    close(pipeline[0]);
			//fprintf(stderr, "Ben Cocugum! - ShiftedInverse\n");
			gettimeofday(&start, NULL);
			det = shiftedInverse2D(matrixINFO.random, matrixINFO.size);
			det1 = determinant(tempMatrix, matrixINFO.size) - det;
			gettimeofday(&end, NULL);
			timedif1 = ((MILLION*(end.tv_sec - start.tv_sec) +
							(end.tv_usec - start.tv_usec)))/1000;
			write(pipeline[1], &det1, sizeof(double));
			write(pipeline[1], &timedif1, sizeof(long));
			close(pipeline[1]);
			
			for (i=0;i<matrixINFO.size/2;i++)
		      free(tempMatrix[i]);
		   free(tempMatrix);
			exit(1);
		}

		close(pipeline[1]);
		read(pipeline[0], &det1, sizeof(double));
		read(pipeline[0], &timedif1, sizeof(long));
		close(pipeline[0]);	

		waitpid(childPID, NULL, WUNTRACED);
			

		

		pipe(pipeline);
		childPID2 = fork();
//sleep(2);
		if(childPID2 == 0)
		{

			tempMatrix = malloc((matrixINFO.size)*sizeof(double *));
			for (i=0;i<matrixINFO.size;i++)
			   tempMatrix[i] = malloc((matrixINFO.size)*sizeof(double));
			
			for(k = 0; k<matrixINFO.size; k++){
		        for(l = 0; l<matrixINFO.size; l++){
		        	tempMatrix[k][l] = matrixINFO.random[k][l];
		        }
		    }

		    close(pipeline[0]);
			//fprintf(stderr, "Ben Cocugum - Convo!\n");
			gettimeofday(&start, NULL);
			det = convolution(matrixINFO.random, matrixINFO.size);
			det2 = determinant(tempMatrix, matrixINFO.size) - det;
			gettimeofday(&end, NULL);
			timedif2 = ((MILLION*(end.tv_sec - start.tv_sec) +
							(end.tv_usec - start.tv_usec)))/1000;

			write(pipeline[1], &det2, sizeof(double));
			write(pipeline[1], &timedif2, sizeof(long));
			close(pipeline[1]);

			for (i=0;i<matrixINFO.size/2;i++)
		      free(tempMatrix[i]);
		   free(tempMatrix);
			exit(1);
		}

		close(pipeline[1]);
		read(pipeline[0], &det2, sizeof(double));
		read(pipeline[0], &timedif2, sizeof(long));
		close(pipeline[0]);

    	waitpid(childPID2, NULL, WUNTRACED);
    	
    	//fprintf(stderr, "res: %lf, time1: %ld, res2: %lf, time2: %ld\n", det1, timedif1, det2, timedif2);

    	i++;

    	if(received)
    	{
    		if(doneflag)
    		{
	    		sprintf(showResFIFO, "%s.temp", specialFIFO);

	    		if(mkfifo(showResFIFO, 0666) == -1){
					perror("Failed to FIFO");
					exit(1);
				}
				fprintf(stderr, "ShowResult + %s\n", showResFIFO);
				fd3 = open(showResFIFO, O_RDWR);
				fprintf(stderr, "fd3: %d\n", fd3);
				doneflag = 0;
    	fprintf(stderr, "res: %lf, time1: %ld, res2: %lf, time2: %ld\n", det1, timedif1, det2, timedif2);

				sendToShow.result1 = det1;
				sendToShow.result2 = det2;
				sendToShow.timeElapsed1 = timedif1;
				sendToShow.timeElapsed2 = timedif2;
				sendToShow.myPID = getpid();
			}
			sigqueue(showPID, SIGUSR2, val);
			fprintf(stderr, "YAZACAM\n");
			check = write(fd3, &sendToShow, sizeof(struct results));
			fprintf(stderr, "YAZDIM\n");
			fprintf(stderr, "showPID: %ld, write-showres-check: %d\n",(long)showPID, check);
			
    	}

	}
	

    return 0;
}

double shiftedInverse2D(double arr[255][255], int size)
{
	int i, j, k, l;
	double ** part1;
	double ** part2;
	double ** part3;
	double ** part4;
	double ** complete;
	double det;
	

/**********************ALLOCATION********************/
	part1 = malloc((size/2)*sizeof(double *));
    for (i=0;i<size/2;i++)
       part1[i] = malloc((size/2)*sizeof(double));

	part2 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part2[i] = malloc((size/2)*sizeof(double));

	part3 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part3[i] = malloc((size/2)*sizeof(double));

	part4 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part4[i] = malloc((size/2)*sizeof(double));

	complete = malloc((size)*sizeof(double *));
	for (i=0;i<size;i++)
	   complete[i] = malloc((size)*sizeof(double));
/*********************ALLOCATION********************/
//fprintf(stderr,"ALLOCATION OK\n");

/**********************DIVISION********************/
	k=0;
	l=0;
	for (i = 0; i < size/2; i++)   
	{
 		for (j = 0; j < size/2; j++)
 		{
 			part1[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "1. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = 0; i < size/2; i++)
	{
		for (j = size/2; j < size; j++)
		{
 			part2[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "2. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = size/2; i < size; i++)
	{
		for (j = 0; j < size/2; j++)
		{
 			part3[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "3. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = size/2; i < size; i++)
	{
        for (j = size/2; j < size; j++)
        {
 			part4[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "4. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
/**********************DIVISION********************/
 	/*
fprintf(stderr,"DIVISION OK\n");


 	fprintf(stderr, "\n--------------------\n");
 	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part1[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
	fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part2[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
    fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part3[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
    fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part4[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
*/
/**********************INVERSE********************/

	cofactor(part1, size/2);
	cofactor(part2, size/2);
	cofactor(part3, size/2);
	cofactor(part4, size/2);

	k=0;
	l=0;
	for(i=0; i<size; i++)
	{
		if(i == (size/2))
		{
			
			k=0;
		}
		l=0;
		for(j=0; j<size; j++)
		{
			if(j == (size/2))
			{
				l=0;
			}
			if((i < size/2) && (j < size/2))
			{
				complete[i][j] = part1[k][l];
			}
			else if((i < size/2) && (j >= size/2))
			{
				complete[i][j] = part2[k][l];
			}
			else if((i >= size/2) && (j < size/2))
			{
				complete[i][j] = part3[k][l];
			}
			else if((i >= size/2) && (j >= size/2))
			{
				complete[i][j] = part4[k][l];
			}
			l++;
			//fprintf(stderr,"%.1lf\t", complete[i][j]);
		}
		k++;
	}/*
	fprintf(stderr, "\n---------COMPLETE-----------\n");
	for(k = 0; k<4; k++){
        for(l = 0; l<4; l++){
        	fprintf(stderr,"%.1lf\t", complete[k][l]);
        }
        fprintf(stderr, "\n\n");
    }
*/
	det = determinant(complete, size);
    for (i=0;i<size/2;i++)
      free(part1[i]);
   free(part1);

   for (i=0;i<size/2;i++)
      free(part2[i]);
   free(part2);

   for (i=0;i<size/2;i++)
      free(part3[i]);
   free(part3);

   for (i=0;i<size/2;i++)
      free(part4[i]);
   free(part4);

   for (i=0;i<size/2;i++)
      free(complete[i]);
   free(complete);

   return det;
//sleep(1);
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
            free(m[i]);
         free(m);
      }
   }
   return(det);
}


void cofactor(double **num, int f)
{

 double d;
 double tmp;
 double** b;
 double** fac;
 int p,q,m,n,i,j;
 

 b = malloc((f)*sizeof(double *));
 	for (i=0;i<f;i++)
     b[i] = malloc((f)*sizeof(double));

 fac = malloc((f)*sizeof(double *));
 	for (i=0;i<f;i++)
     fac[i] = malloc((f)*sizeof(double));

 d = determinant(num,f);

 for (q=0;q<f;q++)
 {
   for (p=0;p<f;p++)
    {
     m=0;
     n=0;
     for (i=0;i<f;i++)
     {
       for (j=0;j<f;j++)
        {
          if (i != q && j != p)
          {
            b[m][n]=num[i][j];
            if (n<(f-2))
             n++;
            else
             {
               n=0;
               m++;
               }
            }
        }
      }
      fac[q][p]=pow(-1,q + p) * determinant(b,f-1);
    }
  }



   for (i=1;i<n;i++) {
      for (j=0;j<i;j++) {
         tmp = fac[i][j];
         fac[i][j] = fac[j][i];
         fac[j][i] = tmp;
      }
   }

  for (i=0;i<f;i++)
    {
     for (j=0;j<f;j++)
       {
        num[i][j]= fac[i][j] / d;
        }
    }


  for (i=0;i<f;i++)
      free(b[i]);
   free(b);

   for (i=0;i<f;i++)
      free(fac[i]);
   free(fac);
}


double convolution(double arr[255][255], int size)
{
	int i, j, k, l;
	double ** part1;
	double ** part2;
	double ** part3;
	double ** part4;
	double ** complete;
	double det;
	

/**********************ALLOCATION********************/
	part1 = malloc((size/2)*sizeof(double *));
    for (i=0;i<size/2;i++)
       part1[i] = malloc((size/2)*sizeof(double));

	part2 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part2[i] = malloc((size/2)*sizeof(double));

	part3 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part3[i] = malloc((size/2)*sizeof(double));

	part4 = malloc((size/2)*sizeof(double *));
	for (i=0;i<size/2;i++)
	   part4[i] = malloc((size/2)*sizeof(double));

	complete = malloc((size)*sizeof(double *));
	for (i=0;i<size;i++)
	   complete[i] = malloc((size)*sizeof(double));
/*********************ALLOCATION********************/
//fprintf(stderr,"ALLOCATION OK\n");

/**********************DIVISION********************/
	k=0;
	l=0;
	for (i = 0; i < size/2; i++)   
	{
 		for (j = 0; j < size/2; j++)
 		{
 			part1[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "1. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = 0; i < size/2; i++)
	{
		for (j = size/2; j < size; j++)
		{
 			part2[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "2. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = size/2; i < size; i++)
	{
		for (j = 0; j < size/2; j++)
		{
 			part3[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "3. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/
 	k=0;
	l=0;
	for (i = size/2; i < size; i++)
	{
        for (j = size/2; j < size; j++)
        {
 			part4[k][l] = arr[i][j];
 			l++;
 			//fprintf(stderr, "4. i: %d\t j: %d\n", i ,j);
 		}
 		k++;
 		l=0;
 	}
 	/**********************************/

/**********************DIVISION********************/
 	/*
fprintf(stderr,"DIVISION OK\n");


 	fprintf(stderr, "\n--------------------\n");
 	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part1[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
	fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part2[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
    fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part3[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
    fprintf(stderr, "\n--------------------\n");
	for(k = 0; k<2; k++){
	        for(l = 0; l<2; l++){
	        	fprintf(stderr,"%lf\t", part4[k][l]);
	        }
	        fprintf(stderr, "\n\n");
	    }
*/
/**********************INVERSE********************/

	convolution_2D(part1, size/2);
	convolution_2D(part2, size/2);
	convolution_2D(part3, size/2);
	convolution_2D(part4, size/2);

	k=0;
	l=0;
	for(i=0; i<size; i++)
	{
		if(i == (size/2))
		{
			
			k=0;
		}
		l=0;
		for(j=0; j<size; j++)
		{
			if(j == (size/2))
			{
				l=0;
			}
			if((i < size/2) && (j < size/2))
			{
				complete[i][j] = part1[k][l];
			}
			else if((i < size/2) && (j >= size/2))
			{
				complete[i][j] = part2[k][l];
			}
			else if((i >= size/2) && (j < size/2))
			{
				complete[i][j] = part3[k][l];
			}
			else if((i >= size/2) && (j >= size/2))
			{
				complete[i][j] = part4[k][l];
			}
			l++;
			//fprintf(stderr,"%.1lf\t", complete[i][j]);
		}
		k++;
	}/*
	fprintf(stderr, "\n---------COMPLETE-----------\n");
	for(k = 0; k<4; k++){
        for(l = 0; l<4; l++){
        	fprintf(stderr,"%.1lf\t", complete[k][l]);
        }
        fprintf(stderr, "\n\n");
    }
*/

	det = determinant(complete, size);
   for (i=0;i<size/2;i++)
      free(part1[i]);
   free(part1);

   for (i=0;i<size/2;i++)
      free(part2[i]);
   free(part2);

   for (i=0;i<size/2;i++)
      free(part3[i]);
   free(part3);

   for (i=0;i<size/2;i++)
      free(part4[i]);
   free(part4);

   for (i=0;i<size/2;i++)
      free(complete[i]);
   free(complete);

   return det;
}

void convolution_2D(double **in, int size){

	int kernel[3][3] = {{0,0,0},
						   {0,1,0},
						   {0,0,0}};
	double **out;
	int i, j, m, n, ii, jj;

	 out = malloc((size)*sizeof(double *));
 	for (i=0;i<size;i++)
     out[i] = malloc((size)*sizeof(double));

	for(i=0; i < size; ++i)              // rows
	{
	    for(j=0; j < size; ++j)          // columns
	    {
	        for(m=0; m < 3; ++m)     // kernel rows
	        {
	            for(n=0; n < 3; ++n) // kernel columns
	            {
	               
	                if( m == 1 && n == 1 ){
	                    out[i][j] = in[i][j] * kernel[m][n];
	                }
	                
	            }
	        }
	    }
	}
	for (i=0;i<size;i++)
    {
     for (j=0;j<size;j++)
       {
        in[i][j]= out[i][j];
        }
    }

	for (i=0;i<size;i++)
      free(out[i]);
   free(out);

}