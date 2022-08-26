#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) {
	std::cout << msg;
	return;
}

int try_connect(int sockfd, struct sockaddr_in serv_addr) {
    std::cout << "Connecting...\n";
    std::cout << "sockfd: " << sockfd << '\n'; 
    std::cout << "serv_addr: ..." << '\n';     
    
    while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  {
        std::cout << "Connection error, retrying...\n"; 
        usleep(1000000);
    }   
	std::cout << "Connected.\n";
	return 0;
}

int main(int argc, char *argv[])
{
    int portno = 21947;
    struct sockaddr_in serv_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { error("ERROR opening socket"); }
        
    struct hostent *server = gethostbyname("localhost");
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
    
    // Соединение с сервером
    try_connect(sockfd, serv_addr);
	
	// Прием данных сервера
	int n;
	char buffer[256];
	while (1) {
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) { error("ERROR reading from socket"); }       
		if (n == 0) { 
		    std::cout << "Connection lost.\n";
 		    break;
 		    //try_connect(sockfd, serv_addr); continue; 
		}       
		std::cout << "n: " << n << '\n';
		printf("Server message: %s\n",buffer);
	}
    
    close(sockfd);
    main(argc, argv);
    return 0;
}
