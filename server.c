#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"

#define PORT 8081

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

typedef struct clientInfo{
	int id;
	char topics[MAX_CLIENTS][MAX_CLIENTS];
	int sf[MAX_CLIENTS];
}clientTCP;

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, sockudp;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;
	
	clientTCP *clienti = (clientTCP *)malloc(MAX_CLIENTS);

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");


	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

//===============================================================//

	sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockudp < 0, "socket");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));

	ret = bind (sockudp, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "bind");

	FD_SET(sockudp, &read_fds);
	fdmax = sockudp;

//===============================================================//
	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}

					printf("New client %d connected from %s:%d.\n",
					newsockfd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

					clienti[i].id = newsockfd;
					
				} 
				else if(i == sockudp){
					memset(buffer, 0, BUFLEN);
					clilen = sizeof(cli_addr);
					n = recvfrom(i, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, &clilen);
					//in buffer se afla payload-ul primit de la UDP
					char auxiliar[BUFLEN];
					//inlocuiesc \0 cu spatiu si sparg buffer in bucati pentru a gasi topicul
					buffer[strlen(buffer)] = ' ';
					char topic[12000];
					char *token = strtok(buffer, " ");
					while(token){
						strcpy(topic, token);
			    		strcpy(auxiliar, token);
			    		strcat(auxiliar, " ");
			    		token = strtok(NULL, " ");
			    		strcat(auxiliar, token);
			    		strcat(auxiliar, " ");
			    		token = strtok(NULL, " ");
			    		strcat(auxiliar, token);
						token = strtok(NULL, " ");
					}
					char mesaj[100000];

					//IP PORT MESAJ
					sprintf(mesaj, "%s %d %s", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), auxiliar);

					for(int j=4 ;j<=fdmax;j++){
                    	if(FD_ISSET(j,&read_fds) && j!=i){
                    		int checkTopic = 0;
                    		int sfPoz = 0;
							for(int y = 0; y < MAX_CLIENTS; y++){
								if(strcmp(clienti[i].topics[y], topic)){
									checkTopic = 1;
									sfPoz = y;
								}
							}
							if(checkTopic == 1 && clienti[i].sf[sfPoz] == 1){
								n = send(j, mesaj, strlen(mesaj), 0);
								DIE(n < 0, "send");

							}
                    	}
                    }
				}
				//de aici se citesc mesajele primite de la subscriber
				else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);

					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						printf("Client %d disconnected.\n", i);
						close(i);
						
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					}
					 
					else {
						//indexul i reprezinta idul clientului
					    char topic[12000];
					    
					    int sf;
					    char* token = strtok(buffer, " ");

					    while(token){
					    	if(strcmp(token, "subscribe") == 0){
					    		token = strtok(NULL, " ");
					    		strcpy(topic, token);
					    		token = strtok(NULL, " ");
					    		sf = atoi(token);

					    		for(int y = 0; y < MAX_CLIENTS; y++){
					    			if( strcmp(clienti[i].topics[y],"") == 0){
					    				strcpy(clienti[i].topics[y], topic);
					    				clienti[i].sf[y] = sf;
					    			}
					    		}
					    	}
					    	else if(strcmp(token, "unsubscribe") == 0){
					    		token = strtok(NULL, " ");
					    		strcpy(topic, token);

					    		int pozitie;
					    		for(int y = 0; y < MAX_CLIENTS; y++){
					    			if( strcmp(clienti[i].topics[y], topic) == 0, && clienti[i].sf[y] == 1){
					    				pozitie = y;
					    			}
					    		}
					    		for(int y = pozitie; y < MAX_CLIENTS-1; y++){
					    			strcpy(clienti[i].topics[y], clienti[i].topics[y+1]);
					    			clienti[i].sf[y] = clienti[i].sf[y+1];
					    		}

					    	}
					    	token = strtok(NULL, " ");
					    }
					}
				}
			}
		}
	}
	close(sockfd);
	return 0;
}
