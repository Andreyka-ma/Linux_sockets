#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) {
	std::cout << msg;
	exit(1);
}

int try_connect(int sockfd, struct sockaddr_in serv_addr) {
    std::cout << "Connecting...\n";     
    while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  {
        std::cout << "Connection error, retrying...\n"; 
        usleep(1000000);
    }   
	std::cout << "Connected.\n";
	return 0;
}

int main(int argc, char *argv[])
{
    std::cout << "CLIENT\n";
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { error("ERROR opening socket"); }
        
    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	// Адрес сервера
    int portno = 21947;
    struct sockaddr_in serv_addr;    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    
    // Соединение с сервером
    try_connect(sockfd, serv_addr);
	
	// Прием данных сервера
	char buffer[256];
	while (1) {
		bzero(buffer,256);
		int n = read(sockfd,buffer,255);
		if (n < 0) { error("ERROR reading from socket"); }       
		if (n == 0) { 
		    std::cout << "Connection lost, trying to reconnect...\n";
 		    close(sockfd);
 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    		if (sockfd < 0) { error("ERROR opening socket"); }
 		    try_connect(sockfd, serv_addr); 
 		    continue; 
		}       
		std::cout << "n: " << n << '\n';
		std::cout << "Server message: " << buffer << '\n';

		// Обратная связь с сервером
		bool lost_con = 0;
		write(sockfd, &lost_con, 1);
	}
    
    close(sockfd);
    return 0;
}
