#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	char clientId[BUFLEN];
	char ipServer[100];
	char portServer[100];
	int check = 0;

	if (argc < 4) {
		usage(argv[0]);
	}
	strcpy(clientId, argv[1]);
	printf("%s\n", clientId);

	strcpy(ipServer, argv[2]);
	printf("%s\n", ipServer);

	strcpy(portServer, argv[3]);
	printf("%s\n", portServer);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(portServer));
	ret = inet_aton(ipServer, &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");
	fd_set read_fds;	// multimea de citire
	fd_set tmp_fds;		// multime temporara 
	int fdmax; 			//maximul socket-urilor 
	
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	//baga socketul de citire in read_fds
	FD_SET(0, &read_fds);
	
	//baga socket-ul serverului in read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
	
	while (1) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

 		if(FD_ISSET(0,&tmp_fds)){

	    	// se citeste de la tastatura

			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			//se trimite mesaj la server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");

			char output[100];
			char *tok = strtok(buffer, " ");
			strcpy(output, tok);
			tok = strtok(NULL, " ");
			strcat(output, "d ");
			strcat(output, tok);
			printf("%s\n", output);

        }
 		else if(FD_ISSET(sockfd,&tmp_fds)){
 		    memset(buffer,0,BUFLEN);
            n = recv(sockfd, buffer, sizeof(buffer), 0);
            buffer[strlen(buffer)] = ' ';
            //inlocuiesc caracterul '\0' cu ' ' 
            DIE(n < 0, "receive");

            printf("%s:%d", ipServer, ntohs(serv_addr.sin_port));

            char out[100000];
            char *token = strtok(buffer, " ");

            strcpy(out, token);
            token = strtok(NULL, " ");
            strcat(out, ":");
            strcat(out, token);
            printf("%s", out);

            while(token){
            	if(*token == '0'){
            		printf(" - %s", "INT");
            		check = 0;
            	}
            	else if(*token == '1'){
            		printf(" - %s", "SHORT_REAL");
            		check = 1;
            	}
            	else if(*token == '2'){
            		printf(" - %s", "FLOAT");
            		check = 2;
            	}
            	else if(*token == '3'){
            		printf(" - %s", "STRING");
            		check = 3;
            	}
            	else{
            		if(check == 0){
            			printf(" - %d", ntohl(*token));
            		}
            		else if(check == 1){
            			printf(" - %d", ntohs(*token));
            		}
            		else if(check == 2){
            			//printf("%s\n", wut?);
            		}
            		else
            			printf(" - %d", *token);
            	}
            	token = strtok(NULL, " ");
            }
            
		}
	}
	close(sockfd);
	return 0;
}
