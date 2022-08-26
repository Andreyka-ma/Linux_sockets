/* A simple server in the internet domain using TCP */
#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
	std::cout << msg;
	exit(1);
}

int main(int argc, char *argv[])
{
    std::cout << "SERVER\n";
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) { error("ERROR opening socket"); }
	
	// Разрешение переиспользования порта
	const int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	    error("setsockopt(SO_REUSEADDR) failed");
	
	// Привязка сокета к адресу
	int portno = 21947;
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		 sizeof(serv_addr)) < 0) 
		 { error("ERROR on binding"); }
	
	listen(sockfd,5);
	
	// Новый сокет и адрес клиента
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	std::cout << "Waiting for acccept\n";
    //std::cout << "sockfd: " << sockfd << '\n'; 
	int newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0) { error("ERROR on accept"); }
	std::cout << "Accepted\n";
	//std::cout << "newsockfd: " << newsockfd << '\n'; 
	
	// Передача данных 
	char buffer[256];
	while(1) {
		bzero(buffer,256);
		std::cout << "Enter a value:\n";
		std::cin >> buffer;
		int n = write(newsockfd,buffer,255);
		//std::cout << "n: " << n << '\n'; 
		//std::cout << "newsockfd: " << newsockfd << '\n'; 
		if (n < 0) { error("ERROR writing to socket"); }
		char c = 'L';
		int nr = read(newsockfd,&c,1);
		std::cout << "Char from client: " << c << '\n';
	}    
	close(newsockfd);
	close(sockfd);
	main(argc, argv);	
	return 0; 
}
