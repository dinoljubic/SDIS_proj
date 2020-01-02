/*
Client queries the Time Daemon and adjusts it's clock according to Berkeley's algorithm 
*/
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
using namespace std;

auto begin = chrono::steady_clock::now();

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

// parametros globales
long sockfd, n;
struct sockaddr_in serv_addr;
struct hostent *server;

void startSync(int portno){
	// beginning start socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0)
		printf("ERROR opening socket");

	server = gethostbyname("localhost");

	if(server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	// end opening
}
int sync(int portno, chrono::time_point<std::chrono::steady_clock> start){
	// parametros locales
	unsigned long l_clock = 0;
	char buffer[256];

	auto end = chrono::steady_clock::now();
	l_clock = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	cout << "My logical Clock: " << l_clock << endl; // Printing machine's local logical clock

	bzero(buffer,256);
	n = read(sockfd,buffer,255);	// Reading Time Daemon's Logical Clock
	if (n < 0)
		error("ERROR reading from socket");

	cout << "Time Daemon Initiating Berkeley's Algorithm!\nThis is TD's logical clock: " << buffer << endl;	// Printing Time Daemon's Logical Clock

	stringstream ss, ss1;

	ss << buffer;
	string tmpstr1 = ss.str();

	unsigned long tmp = atol(tmpstr1.c_str());	// converting Time Daemon's clock from char array to integer value

	unsigned long diff = l_clock - tmp;		// Calculating time difference of local machine from Time Daemon
	cout << "My Time Difference from TD: "<< diff << endl;

	bzero(buffer,256);

	ss1 << diff;
	string tmpstr2 = ss1.str();
	strcpy(buffer,tmpstr2.c_str());

	n = write(sockfd,buffer,strlen(buffer));	// Sending this machine's Time Difference to Time Daemon
	if (n < 0)
		error("ERROR writing to socket");

	bzero(buffer,256);
	n = read(sockfd,buffer,255);			// Reading the final average value to be adjusted in local machine's logical clock
	printf("Clock Adjustment= %s\n",buffer);

	unsigned long adj_clock = atoi(buffer);

	l_clock = l_clock + adj_clock;

	cout << "My Adjusted clock: " << l_clock << endl;

	return l_clock;
}

void closeSync(){
	close(sockfd);	//Close the client socket and terminate the connection
}

int main(int argc, char *argv[])
{

	if(argc < 2) {
		fprintf(stderr,"usage %s port\n", argv[0]);
		exit(0);
	}
	
	chrono::time_point<std::chrono::steady_clock> start = chrono::steady_clock::now();
	int portno = atoi(argv[1]);

	for (int i = 0; i < 5; ++i) {
		startSync(portno);
		cout << "Iteracao num: " << i << endl;
		sync(portno, start);
		closeSync();
		// sleep(1);
	}



	return 0;
}
