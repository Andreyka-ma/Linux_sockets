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
	int try_connect();
	
	// Метод для определения количества символов
	int symbols(int n) const;
	
private:
	int sockfd;
	struct hostent *server;
	struct sockaddr_in serv_addr;
};

int main(int argc, char *argv[]) {
	int port_num = 21947;
	Prog2_Cli C(port_num);
	return 0; 
}

Prog2_Cli::Prog2_Cli(int portno) {
	std::cout << "Prog_2.\n";
	// Используется TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// localhost
	server = gethostbyname("localhost");
	
	// Адрес сервера    
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	bcopy((char *)server->h_addr, 
	      (char *)&serv_addr.sin_addr.s_addr,
	      server->h_length);

	// Соединение с программой 1
	try_connect();
	
	// Прием данных программы 1
	while (1) {
		int value = -1;
		read(sockfd,&value,sizeof(int));
		if (value == -1) {
			// Потеряно соединение
			std::cout << "Prog_1 connection lost.\n";
 		    close(sockfd);
 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
 		    try_connect(); 
 		    continue;			 
		} 
		// Значение value -2 не обрабатываем, т.к.
		// оно лишь означает, что соединение функционирует
		else if (value != -2) {
			// Анализ количества символов
			int value_sym = symbols(value);
			if (value_sym <= 2) { 
				// Выводим сообщение об ошибке
				std::cout << "Error - received a " << value_sym
				<< "-symbol value (more than 2 symbols required).\n";
			}
			else if (value % 32 != 0) {
				// Выводим сообщение об ошибке
				std::cout << "Error - received value is not a multiple of 32.\n";
			}
			else {
				// Выводим сообщение о данных
				std::cout << "Received data from Prog_1: " << value << '\n';
			}
		}
		
		// Обратная связь с программой 1
		bool lost_con = 0;
		write(sockfd, &lost_con, 1);
	}	
}

Prog2_Cli::~Prog2_Cli() { close(sockfd); }

// Метод для переподключения к программе 1
int Prog2_Cli::try_connect() {
	std::cout << "Waiting for Prog_1 connection...\n";     
	while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  {
	    sleep(0.5);
	}   
	std::cout << "Connected.\n";
	return 0;
}

// Метод для определения количества символов.
int Prog2_Cli::symbols(int n) const { 
	int cnt = 0;
	while(n > 0) {
		n /= 10;
		cnt++; 
	}	
	return cnt; 
}

