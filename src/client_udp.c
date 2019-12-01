/* Ce client UDP : 
  1/ prend en parametre une adresse IP (exemple d'utilisation : ./client_udp 127.0.0.1)
  2/ envoie un paquet UDP avec le mot "request" a un serveur ecoutant sur le port 3000 et sur l'adresse indiquee en parametre
  3/ lit la reponse , affiche celle ci , avec le port et l'adresse de l'emetteur de la reponse 
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>


/* definition des constantes */
#define MAXLINE 1024
#define MAX_LIMIT 1024
#define LEN 150

int main(int argc, char *argv[])
{
   // while(1){
       
     int sock;
     int  nread, nsend;
     struct sockaddr_in addr;
     char buffer[MAXLINE+1];

     /* SOCKET creation de socket */
     if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
         perror("Socket creation error");
         return -1;
     }
     
     /* SEND initialiser l’adresse du serveur  */
     /* type d’adresse  INET */
     addr.sin_family = AF_INET;
     addr.sin_port = htons(atoi(argv[2]));
     
     /* contruction d’adresse IP binaire avec inet_pton */
     /* l’adresse est donnee en argument « x.x.x.x » */
     if((inet_pton(AF_INET,argv[1],&addr.sin_addr)) <= 0) {
         perror("Address creation error");
         return -1;
     }
     bzero(addr.sin_zero, 8);
     
     /* envoi d’un packet avec le mot request a l'interieur */
     
     
     printf("\033[1;31mEnter the message at %s:\033[0;0m\t",buf);
     char toSend[MAX_LIMIT]; 
     fgets(toSend, MAX_LIMIT, stdin); 
     printf("-- To \t[\033[1;34m%s\033[0;0m:\033[1;32m%s\033[0;0m] --> %s",argv[1],argv[2],toSend);
     nsend = sendto(sock, toSend, 7, 0, (struct sockaddr *)&addr, sizeof(addr));
     if (nsend < 0) {
         perror("Request error");
         return -1;
     }
     
     /* RECEIVE lecture de la reponse */
     struct sockaddr_in from;
     unsigned int fromlen = sizeof(from);
     nread = recvfrom(sock, buffer, MAXLINE, 0, (struct sockaddr *) &from,&fromlen);
     if (nread < 0) {
         perror("Read error");
         return -1;
     }
     
     /* affichage de la reponse */
     buffer[nread]='\0';
     printf("-- From [\033[1;34m%s\033[0;0m:\033[1;32m%d\033[0;0m] --> %s \n",inet_ntoa(from.sin_addr),ntohs(from.sin_port),buffer);
    //}
    return 0; 
}