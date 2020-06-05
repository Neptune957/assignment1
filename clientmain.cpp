#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Included to get the support library
#include <calcLib.h>
#include "protocol.h"

#define PORT 4950	// the port users will be connecting to
#define MAXDATASIZE 1500


int main(int argc, char *argv[]){
  
  	/* Do magic */
	int sockfd,num;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in server;

	int numbytes;  

	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	  	printf("Error: argument number wrong. dir: %s argument num: (%d)\n",argv[0],argc);
	  	exit(1);
	}

	printf("client: getting server info\n");
	if((he=gethostbyname(argv[1]))==NULL){
		printf("Error: client cannot get server host info\n");
		exit(1);
	}

	printf("client: creating a socket\n");
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("Error: client cannot create a socket\n");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr=*((struct in_addr*)he->h_addr);

	printf("client: connecting to server\n");
	if(connect(sockfd,(struct sockaddr*)&server,sizeof(server))==-1){
		printf("Error: client cannot connect to server\n");  
       	exit(1);
	}

	printf("client: establish a connection to server %s:%d\n",inet_ntoa(server.sin_addr),htons(server.sin_port));

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
	  	printf("Error:client cannot receive data from server\n");
	  	exit(1);
	}
	
	buf[numbytes] = '\0';
	
	printf("server: support communicating protocols as follows:\n\"%s\"\n",buf);
	char *serverSupportProtocol;
	bool supportServerProtocol=false;

	serverSupportProtocol = strtok(buf, "\n");
   
   	while( serverSupportProtocol != NULL ) {
     	if(strcmp(serverSupportProtocol,"TEXT TCP 1.0")==0){
     		char *response="OK";
     		send(sockfd, response, sizeof(response), 0);
     		supportServerProtocol=true;
     		printf("client: OK, achieve agreement on communicating protocol with server\n");
     		break;
     	}
      	serverSupportProtocol = strtok(NULL, "\n");
   	}
   	if(!supportServerProtocol){
   		printf("Client do not support server-side communicating protocol\n");
   		send(sockfd, "Not OK", 7, 0);
   		close(sockfd);
   		exit(1);
   	}

   	memset(buf,0,sizeof(buf));
   	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
	  	printf("Error in receiving data from server\n");
	  	close(sockfd);
	  	exit(1);
	}
	char *operation;
	operation = strtok(buf, " ");
	double fresult;
	int result;
	char answer[100];
	if(operation[0]=='f'){
		double value1=atof(strtok(NULL, " "));
		double value2=atof(strtok(NULL, " "));
		if(operation[1]=='a'){
			fresult=value1+value2;
		}else if(operation[1]=='s'){
			fresult=value1-value2;
		}else if(operation[1]=='m'){
			fresult=value1*value2;
		}else{
			fresult=value1/value2;
		}
		sprintf(answer,"%8.8g\0",fresult);
		printf("client: receive command \"%s %f %f\" from server\nclient: send answer %s back to server\n",operation,value1,value2,answer);
	}else{
		int value1=atoi(strtok(NULL, " "));
		int value2=atoi(strtok(NULL, " "));
		if(operation[0]=='a'){
			result=value1+value2;
		}else if(operation[0]=='s'){
			result=value1-value2;
		}else if(operation[0]=='m'){
			result=value1*value2;
		}else{
			result=value1/value2;
		}
		sprintf(answer,"%d\0",result);
		printf("client: receive command \"%s %d %d\" from server\nclient: send answer %s back to server\n",operation,value1,value2,answer);
	}
	send(sockfd, answer, sizeof(answer), 0);

	memset(buf,0,sizeof(buf));
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
	  	printf("Error in receiving data from server\n");
	  	close(sockfd);
	  	exit(1);
	}
	printf("server: the right answer should be %s\n",buf);

	memset(buf,0,sizeof(buf));
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
	  	printf("Error in receiving data from server\n");
	  	close(sockfd);
	  	exit(1);
	}
	printf("server: %s\n",buf);


	close(sockfd);
	printf("client: program shutdown.\n");


	return 0;

}
