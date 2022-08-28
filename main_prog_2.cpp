/* A simple server in the internet domain using TCP */
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

bool check(int n) {	return (n > 9) && (n % 32 == 0); }

// Функция для переподключения к программе 1
int try_connect(int sockfd, struct sockaddr_in serv_addr) {
    std::cout << "Waiting for Prog_1 connection...\n";     
    while (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  {
        usleep(1000000);
    }   
	std::cout << "Connected.\n";
	return 0;
}

int main(int argc, char *argv[])
{
	std::cout << "Prog_2.\n";
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // localhost
    struct hostent *server = gethostbyname("localhost");
	
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
	
	// Прием данных программы 1
	while (1) {
		int value = -1;
		read(sockfd,&value,sizeof(int));
		if (value == -1) {
			std::cout << "Prog_1 connection lost, trying to reconnect...\n";
 		    close(sockfd);
 		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
 		    try_connect(sockfd, serv_addr); 
 		    continue;			 
		}
		else {
			// Выводим сообщение о полученных данных 
			// если полученное значение состоит из 
			// более 2-ух символов и кратно 32  
			if (check(value)) {
				std::cout << "Received data from Prog_1: " << value << '\n';
			}
			else {
				std::cout << "Error - incorrect data received.\n";
			}
		}

		// Обратная связь с программой 1
		bool lost_con = 0;
		write(sockfd, &lost_con, 1);
	}
	close(sockfd);
	return 0; 
}
