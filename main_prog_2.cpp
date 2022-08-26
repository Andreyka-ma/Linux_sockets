/* A simple server in the internet domain using TCP */
#include <iostream>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int try_accept(int sockfd,	struct sockaddr_in cli_addr, socklen_t clilen) {
	std::cout << "Waiting for acccept\n";
	int newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	std::cout << "Accepted\n";
	return newsockfd;
}

int main(int argc, char *argv[])
{
    std::cout << "SERVER\n";
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// Разрешение переиспользования порта
	const int enable = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
	// Привязка сокета к адресу
	int portno = 21947;
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	bind(sockfd, (struct sockaddr *) &serv_addr,
		 sizeof(serv_addr)); 
	
	listen(sockfd,5);
	
	// Новый сокет и прием клиента
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = try_accept(sockfd, cli_addr, clilen); 
	
	// Передача данных 
	char buffer[256];
	while(1) {
		// Обратная связь с клиентом
		bool lost_con = 0; 
		int n = write(newsockfd,&lost_con,1);
		
		int value = -1;
		read(newsockfd,&value,1);
		if (value == -1) {
			std::cout << "Client connection lost, reconnecting...\n";
			newsockfd = try_accept(sockfd, cli_addr, clilen); 
		}
	}    
	close(newsockfd);
	close(sockfd);
	return 0; 
}


bool check(int n) {
	return (n > 9) && (n % 32 == 0);
}

int main() {
	int n;
	// Получение значения из программы 1
	std::cout << "Waiting for input...\n";
	std::cin >> n;
	// Выводим сообщение о полученных данных 
	// если полученное значение состоит из 
	// более 2-ух символов и кратно 32  
	if (check(n)) {
		std::cout << "Received data: " << n << '\n';
	}
	else {
		std::cout << "Error - incorrect data\n";
	}

	main();
	setlocale(LC_ALL, "rus");
	return 0;
}
