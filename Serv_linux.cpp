/* A simple server in the internet domain using TCP */
#include <iostream>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
	std::cout << msg;
	exit(1);
}

int try_accept(int sockfd,	struct sockaddr_in cli_addr, socklen_t clilen) {
	std::cout << "Waiting for acccept\n";
	int newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0) { error("ERROR on accept"); }
	std::cout << "Accepted\n";
	return newsockfd;
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
	
	// Новый сокет и прием клиента
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = try_accept(sockfd, cli_addr, clilen); 
	
	// Передача данных 
	char buffer[256];
	while(1) {
		bzero(buffer,256);
		std::cout << "Enter a value:\n";
		std::cin >> buffer;
		int n = write(newsockfd,buffer,255);
		if (n < 0) { error("ERROR writing to socket"); }
		
		// Обратная связь с клиентом
		bool lost_con = 1;
		read(newsockfd,&lost_con,1);
		if (lost_con) {
			std::cout << "Client connection lost, reconnecting...\n";
			newsockfd = try_accept(sockfd, cli_addr, clilen); 
		}
	}    
	close(newsockfd);
	close(sockfd);
	main(argc, argv);	
	return 0; 
}
