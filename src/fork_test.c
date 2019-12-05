#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 


void forkexample() { 
	// child process because return value zero 
	if (fork() == 0) 
		printf("Hello from Child! PID = %d\n",getpid()); 

	// parent process because return value non-zero. 
	else
		printf("Hello from Parent! PID = %d\n",getpid()); 
} 


int main(int argc, char const *argv[]){
  forkexample(); 
	return 0; 
}

