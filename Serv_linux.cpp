/* A simple server in the internet domain using TCP */
#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, n;
     int portno = 21947;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) { error("ERROR opening socket"); }
     
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
     		 sizeof(serv_addr)) < 0) 
     		 { error("ERROR on binding"); }
              
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     std::cout << "Process blocked\n";
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) { error("ERROR on accept"); }
     std::cout << "Unblocked\n";
     
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) { error("ERROR reading from socket"); }
     
     std::cout << "Here is the message: " << buffer << '\n';

     n = write(newsockfd,"I got your message",18);
     if (n < 0) { error("ERROR writing to socket"); }
     
     close(newsockfd);
     close(sockfd);
     return 0; 
}
