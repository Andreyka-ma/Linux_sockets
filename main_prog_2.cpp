/* A simple server in the internet domain using TCP */
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

class Prog2_Cli {
public:
	Prog2_Cli(int portno);
	~Prog2_Cli();
	
	// Метод для переподключения к программе 1
	int try_connect(int sockfd, struct sockaddr_in serv_addr);
	
	// Метод проверки принимаемых программой данных
	bool check(int n);
	
private:
	int sockfd;
};

int main(int argc, char *argv[]) {
	int port_num = 21947;
	Prog2_Cli C(port_num);
	return 0; 
}

Prog2_Cli::Prog2_Cli(int portno) {
	std::cout << "Prog_2.\n";
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// localhost
	struct hostent *server = gethostbyname("localhost");
	
	// Адрес сервера
	struct sockaddr_in serv_addr;    
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	bcopy((char *)server->h_addr, 
	      (char *)&serv_addr.sin_addr.s_addr,
	      server->h_length);

	// Соединение с сервером
	try_connect(sockfd, serv_addr);
	
	// Прием данных программы 1
	while (1) {
		int value = -1;
		read(sockfd,&value,sizeof(int));
		if (value == -1) {
			// Потеряно соединение
			std::cout << "Prog_1 connection lost, trying to reconnect...\n";
 		    close(sockfd);
 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
 		    try_connect(sockfd, serv_addr); 
 		    continue;			 
		} 
		// Значение value -2 не обрабатываем, т.к.
		// оно лишь означает, что соединение функционирует.
		else if (value != -2) {
			if (check(value)) {
				// Выводим сообщение о данных 
				// если value состоит из более  
				// 2-ух символов и кратно 32  
				std::cout << "Received data from Prog_1: " << value << '\n';
			}
			else {
				// Выводим сообщение об ошибке
				std::cout << "Error - incorrect data received.\n";
			}
		}
		
		// Обратная связь с программой 1
		bool lost_con = 0;
		write(sockfd, &lost_con, 1);
	}	
}

Prog2_Cli::~Prog2_Cli() { close(sockfd); }

// Метод для переподключения к программе 1
int Prog2_Cli::try_connect(int sockfd, struct sockaddr_in serv_addr) {
	std::cout << "Waiting for Prog_1 connection...\n";     
	while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  {
	    sleep(0.5);
	}   
	std::cout << "Connected.\n";
	return 0;
}

// Метод проверки принимаемых программой данных
bool Prog2_Cli::check(int n) {	return (n > 9) && (n % 32 == 0); }

