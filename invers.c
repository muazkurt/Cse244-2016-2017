//created by Dr. Ramsey on 2/26/15
// demonstration of shared memory issues 
// along with other important pieces

#include <sys/shm.h> //for shm 
#include <stdio.h> //printf
#include <stdlib.h> //included for exit

struct my_object {
  float base;
  int exponent;
};


void error(char* msg) {
  //error in a system call occurred
  perror(msg);
  exit(1); //quit the program
}


//main can have arguments
//the first argument to main is the number of command line arguments
//when you type ls -al, there are 'two' command line arguments
//the first is "ls" and the second is "-al"
//When you make your own programs you typically type ./hw1
//However, in the upcoming assignment, you will 
//use the argv cstring array to handle arguments.


int main(int argc, char *argv[]) {

  int delete_shm = 0;
  if(argc > 1) {
    //there was at least one argument, try to convert the second to an int
    delete_shm = atoi(argv[1]);
  }

  printf("This program was executed as: ");
  for(int i = 0; i < argc; i++) 
    printf("%s ",argv[i]);    //string s = argv[i];
  printf("\n\n");


  int key = 99999; // IPC_PRIVATE for some other guarantees
  int segment_id; //the segment identifier of the shm segment

  //step 1, ask the OS to create the shared memory and get the segment id
  //we use shmget for this : shmget man page http://linux.die.net/man/2/shmget
  //IPC_CREAT says to create it, but don't fail if it is already there
  //IPC_CREAT | IPC_EXCL says to create and fail if it already exists
  // | 0755 uses the octal permissions similar to chmod
  segment_id = shmget(key, sizeof(my_object), IPC_CREAT | 0775);


  if(segment_id == -1 )   error("shmget: ");

  //just for information, we print this out, but not required
  printf("My segment id is %d\n", segment_id);

  
  //step 2 - attach a piece of data in your program to that data segment

  //in this case, I've created the data segment to be the size of 'my_object'
  //so I'm going to attach the data segment to a pointer to that kind of object

  //for this we use shmat http://linux.die.net/man/2/shmat

  my_object* ptr = (my_object*) shmat(segment_id, NULL, 0);
  //we can use shmat(segment_id, NULL, SHM_RDONLY) to attach read-only
  if(ptr == (void*)-1)  error("shmat: ");

  printf("My ptr address is %x\n",ptr);

  //at this point, we can use the my_object pointer to get
  //access to the shared memory
  printf("The base is %f and the exponent is %d\n",
	 ptr->base, ptr->exponent);

  //we can also use ptr as if it is an array name to an
  //array of objects. Be careful not to blow array bounds
  printf(" The base is %f and the exp is %d\n",
 	  ptr[0].base, ptr[0].exponent);


  //hey, let's let the user input these values
  printf("Input a float and an int: " );

  //feel free to use cout and cin in your programs
  scanf("%f%d",&(ptr->base), &(ptr->exponent));

  //and now let's print it again
  printf("The base is %f and the exponent is %d\n",
	 ptr->base, ptr->exponent);


  //detaching from a segment is easy.
  int detach_return = shmdt(ptr);
  if(detach_return == -1)  error("shmdt: ");

  //after detaching form a segment, we can destroy it if we want
  //use shmctl for this: http://linux.die.net/man/2/shmctl


  //we're only going to do this part if the
  //command line arguments tell us we're supposed to
  if(delete_shm == 1) {
    int ctl_return = shmctl(segment_id, IPC_RMID, NULL);
    if(ctl_return == -1)  error("shmctl for removal: ");
  }

}