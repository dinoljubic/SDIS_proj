/* Time Daemon sends it's time to all connected nodes to adjust average time 
   The port number is passed as an argument */
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

using namespace std;

// chrono::steady_clock begin;

void error(const char *msg) //function to print error messages
{
  perror(msg);
  exit(1);
}

char buffer[256];
int n;

int cnt = 0, t=0;
unsigned long l_clock = 0, tot = 0, avg = 0;

long sockfd, newsockfd[10]; //, portno;
socklen_t clilen;

struct sockaddr_in serv_addr, cli_addr;

void Berkeley(long newsockfd)
{
  bzero(buffer,256);

  stringstream ss, ss1, ss2;
  ss << l_clock;
  string tmpstr1 = ss.str();        // Converting clock value from int to string or char array
  strcpy(buffer, tmpstr1.c_str());  // Now converting from string to const char *
  
  n = write((long)newsockfd,buffer,strlen(buffer)); // Sending Time Daemon's logical clock to connected machine
  if (n < 0) error("ERROR writing to socket");

  bzero(buffer,256);
  read((long)newsockfd,buffer,255);     // Reading the specific time difference of connected machines
  cout << "Time Difference of Machine '" << newsockfd << "' : " << buffer << endl;
  
  ss1 << buffer;
  string tmpstr2 = ss1.str();
  
  unsigned long diff = atoi(tmpstr2.c_str()); 

  tot = tot + diff;     //Adding all time differences

  sleep(2);

  avg = tot/(cnt+1);      //Taking average of the total time differences

  unsigned long adj_clock = avg - diff;   //Calculating the average time adjustment for each clock

  bzero(buffer,256);
  
  ss2 << adj_clock;
  string tmpstr3 = ss2.str();
  strcpy(buffer,tmpstr3.c_str());   //Converting time adjustment value from integer to const char *
  n = write((long)newsockfd,buffer,strlen(buffer)); //Sending specific time adjustment to corresponding machine
  if (n < 0) error("ERROR writing to socket");
}


void *NewConnection(void *newsockfd) //thread function for each client request
{
  if((long)newsockfd < 0) 
    error("ERROR on accept");

  
  cout << "Connected to Machine Number: " << (long)newsockfd << endl;
  cnt++;

  while(cnt != t)
  {
    continue;
  }

  Berkeley((long) newsockfd);

  close((long) newsockfd);
  pthread_exit(NULL);
}

void startSync(int portno){
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) //To reuse socket address in case of crashes and failures
    perror("setsockopt(SO_REUSEADDR) failed");

#ifdef SO_REUSEPORT
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0)
    perror("setsockopt(SO_REUSEPORT) failed");
#endif

  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
}



void sync(int portno, chrono::time_point<std::chrono::steady_clock> start){

  pthread_t threads[10];  //threads for handling client requests
  srand(time(0));           // Initiating the random function with current time as input
  auto end = chrono::steady_clock::now();

  l_clock = chrono::duration_cast<chrono::milliseconds>(end - start).count();        // Defining the range of random numbers from 5 to 30

  listen(sockfd, 10);
  clilen = sizeof(cli_addr);

  // cout << "Enter the total number of machines to connect: " ;
  //  cin >> t;
  t = 1;

  cout << "Time Daemon's Logical Clock: " << l_clock << endl;

  int sock_index = 0;

  for(int i=0; i < t; i++)
  {
    newsockfd[sock_index] = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

    pthread_create(&threads[i], NULL, NewConnection, (void *)newsockfd[sock_index]);

    sock_index=(sock_index+1)%100;
  }

  for(int i=0; i<t; i++)
  {
    int rc = pthread_join(threads[i], NULL);
    if(rc)
    {
      printf("Error in joining thread :\n");
      cout << "Error: " << rc << endl;
      exit(-1);
    }

  }

  cout << "Clock adjustment: " << avg << endl;
  l_clock = l_clock + avg;

  cout << "My Adjusted clock: " << l_clock << endl;


}

void closeSync(){
  close(sockfd);
  //  return 0;
  pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
  chrono::time_point<std::chrono::steady_clock> start = chrono::steady_clock::now();

  int portno = atoi(argv[1]);
  startSync(portno);
  for (int i = 0; i < 10; ++i) {
    cnt = 0;
    cout << "Iteracao num: " << i << endl;
    sync(portno, start);
    // sleep(1);
  }
  closeSync();
  // auto start = chrono::steady_clock::now();

  // do some stuff here
  // sleep(3);

  // auto end = chrono::steady_clock::now();

  // cout << "Elapsed time in nanoseconds : " 
  //   << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
  //   << " ns" << endl;

}