#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

//# share profile with server
#include "define.h"

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[SIZE_MESSAGE],aborter[30];
    
    if(argc!=3){
            printf("Usage : %s <IP> <port>\n", argv[0]);
            exit(1);
        }
    
    sock=socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
        
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
        error_handling("connect() error!");
    
    // int str_len= read(sock, message, sizeof(message));
    // if(str_len==-1)
    // error_handling("read() error!");

    // printf("Message from server: %s \n", message); 

    while(1) {
        char message[SIZE_MESSAGE] = "";
        fputs ("if you want to quit, press q or Q : ", stdout);
        fgets (message, SIZE_MESSAGE, stdin);

        
        if (strcmp(message, "q\n") == 0) break;
        write(sock, message, sizeof(message));

        int str_len= read(sock, message, sizeof(message));
        if(str_len==-1)
        error_handling("read() error!");

        printf("Message from server: %s \n", message);  
        
    }

   close(sock);
   return 0;
}

void error_handling(char *message)
{
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}
