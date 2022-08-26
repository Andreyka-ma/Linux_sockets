#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
	std::cout << msg;
	return;
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    int portno = 21947;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
        
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    
    while (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  {
        printf("Connection error, retrying...\n"); 
        usleep(1000000);
    }
	   
	std::cout << "Connected.\n";
	   
	while (1) {
		char val;
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) { error("ERROR reading from socket"); }       
		if (n == 0) { break; }       
		std::cout << "n: " << n << '\n';
		printf("Server message: %s\n",buffer);
	}
    
    close(sockfd);
    main(argc, argv);
    return 0;
}
