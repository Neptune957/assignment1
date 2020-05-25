#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
/* You will to add includes here */


// Included to get the support library
#include <calcLib.h>

#include "protocol.h"
#define PORT "4950"  // the port users will be connecting to

#define BACKLOG 1  // how many pending connections queue will hold


int main(int argc, char *argv[]){
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct sockaddr_in server;
  struct sockaddr_in client;
  socklen_t addrlen;

  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
    perror("server: socket");
    printf("server: bind socket error\n");
    exit(1);
  }

  printf("server: setting socket options\n");
  int opt=SO_REUSEADDR;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt)) == -1) {
    printf("server: set socket error\n");
    perror("setsockopt");
    exit(1);
  }

  bzero(&server,sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY); 

  printf("server: binding socket\n");
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    perror("server: bind");
    printf("server: bind socket error\n");
    exit(1);
  }

  printf("server: listening to socekt\n");
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    printf("server:fail to listen to the socket\n");
    exit(1);
  }
  addrlen=sizeof(client);


  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    printf("server: fail to bind the socket\n");
    exit(1);
  }

  printf("server: waiting for connection...\n");
  char msg[1500];
  int MAXSZ=sizeof(msg)-1;
  
  int readSize;

  while(1) {  // main accept() loop
    if((new_fd=accept(sock_fd,(struct sockaddr*)&client,&addrlen))==-1){
      perror("accept");
      printf("server: accept connection error\n");
      continue;
    }
    printf("server: establish a connection from client %s:%d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));
 
    printf("server: Sending support communication protocols \n");
    if (send(new_fd, "TEXT TCP 1.0\n", 14, 0) == -1){
      perror("send");
      printf("server: send error\n");
      shutdown(new_fd, SHUT_RDWR);
      close(new_fd);
      continue; //leave loop execution, go back to the while, main accept() loop. 
    }
    while(1){
      readSize=recv(new_fd,&msg,MAXSZ,0);
      printf("client: send status \"%s\"\n",msg);

      msg[readSize]='\0';
      if(strcmp(msg,"OK")!=0){
        printf("error:client does not support server\'s communication protocols\n");
        shutdown(new_fd, SHUT_RDWR);
        close(new_fd);
        break;
      }

      initCalcLib();
      char operationCommand[MAXSZ];
      char *operation=randomType();
      double fresult=0;
      int result=0;
      if(operation[0]=='f'){
        double value1=randomFloat(),value2=randomFloat();
        if(operation[1]=='a'){
          fresult=value1+value2;
        }else if(operation[1]=='s'){
          fresult=value1-value2;
        }else if(operation[1]=='m'){
          fresult=value1*value2;
        }else{
          fresult=value1/value2;
        }
        sprintf(operationCommand,"%s %f %f\0",operationCommand,value1,value2);
      }else{
        int value1=randomInt(),value2=randomInt();
        if(operation[0]=='a'){
          result=value1+value2;
        }else if(operation[0]=='s'){
          result=value1-value2;
        }else if(operation[0]=='m'){
          result=value1*value2;
        }else{
          result=value1/value2;
        }
        sprintf(operationCommand,"%s %d %d\0",operationCommand,value1,value2);
      }
      printf("server: send \"%s\" to client",operationCommand);

      if (send(new_fd, operationCommand, MAXSZ, 0) == -1){
        perror("send");
        printf("server: send error\n");
        shutdown(new_fd, SHUT_RDWR);
        close(new_fd);
        continue; //leave loop execution, go back to the while, main accept() loop. 
      }

      memset(msg,0,MAXSZ);
      readSize=recv(new_fd,&msg,MAXSZ,0);
      char *finalResponse;
      char resultString[MAXSZ];

      printf("client: my answer to \"operationCommand\" is: %s\n",msg);
      if(operation[0]=='f'){
        sprintf(resultString,"%8.8g\0",fresult);
      }else{
        sprintf(resultString,"%d\0",result);
      }

      if(strcmp(msg,resultString)==0){
        finalResponse="OK";
        printf("server: right answer\n");
      }else{
        finalResponse="ERROR";
        printf("server: wrong answer\n");
      }
      send(new_fd, finalResponse, sizeof(finalResponse), 0);
      shutdown(new_fd, SHUT_RDWR);
      close(new_fd);
      printf("server: mission has done. stop connection.\n");
      break;
    }
  }

  return 0;
}