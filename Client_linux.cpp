#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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

    
    
	int error = 0;
	socklen_t len = sizeof (error);
	int retval = getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

	if (retval != 0) {
    /* there was a problem getting the error code */
    fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
	}

	if (error != 0) {
		/* socket has a non zero error status */
		fprintf(stderr, "socket error: %s\n", strerror(error));
	}

	// Соединение с сервером
    try_connect(sockfd, serv_addr);
	

	// Прием данных сервера
	char buffer[256];
	while (1) {
		bzero(buffer,256);
		int n = read(sockfd,buffer,255);
		if (n == 0) { 
		    std::cout << "Connection lost, trying to reconnect...\n";
 		    close(sockfd);
 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
