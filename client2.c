#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h>

// #include <WinSock2.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE 100
#include "define.h"

int main(int argc, char *argv[])
{
        /////////////////////////////////////////////
        //multiplex 서버로서의 소켓

        int serv_sock, clnt_sock;
        struct sockaddr_in serv_adr, clnt_adr;
        struct timeval timeout;
        fd_set reads;

        socklen_t adr_sz;
        int fd_max, str_len, fd_num, sock;
        char buf[BUF_SIZE];
        int on = 1;

        char myNickname[20];

        if (argc != 2)
        {
                printf("Usage : %s <port>\n", argv[0]);
                exit(1);
        }

        serv_sock = socket(PF_INET, SOCK_STREAM, 0);
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_adr.sin_port = htons(atoi(argv[1]));

        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

        if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        {
                printf("bind() error");
                exit(0);
        }
        if (listen(serv_sock, 5) == -1)
        {
                printf("listen() error");
                exit(0);
        }
        /////////////////////////////////////////////
        /////////////////////////////////////////////
        //클라이언트로서의 소켓
        int sock2;
        struct sockaddr_in serv_addr2;
        char message[BUF_SIZE], aborter[30];
        sock2 = socket(PF_INET, SOCK_STREAM, 0);
        if (sock2 == -1)
                printf("socket() error");

        memset(&serv_addr2, 0, sizeof(serv_addr2));
        serv_addr2.sin_family = AF_INET;
        serv_addr2.sin_addr.s_addr = inet_addr("127.0.0.1");
        serv_addr2.sin_port = htons(atoi("9100"));

        if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
                printf("connect() error!");

        /////////////////////////////////////////////

        FD_ZERO(&reads);
        FD_SET(serv_sock, &reads);
        fd_max = serv_sock;

        /////////////////////////////////////////////
        //로그인
        printf("%s\n", "닉네임을 입력하세요~");
        scanf("%s", myNickname);

        commandObject loginCommandObject;
        char loginCommand[100];
        loginCommandObject.commandCode = COMMAND_LOGIN;
        strcpy(loginCommandObject.argument, myNickname);

        generateCommandOptionString(&loginCommandObject, loginCommand);

        write(sock2, loginCommand, sizeof(loginCommand)); // echo!

        printf("Request Body : %s\n", loginCommand);

        char responseCommand[100];
        str_len = read(sock2, responseCommand, sizeof(responseCommand));
        if (str_len == -1)
                printf("read() error!");

        printf("%d", str_len);

        // printf("Response Body: %s\n", responseCommand);
        // fflush(0);
        // for (int i = 0; i < 10; i++)
        // {
        //         printf("%s", "*");
        // }

        fflush(0);

        // while(1)
        // {
        //         FD_SET(serv_sock, &reads);
        //         timeout.tv_sec=5;
        //         timeout.tv_usec=5000;

        //         if((fd_num=select(fd_max+1, &reads, 0, 0, &timeout))==-1)
        //                 break;

        //         if(fd_num==0)
        //                 printf(" Timeout \n");

        //         for(sock=0; sock<fd_max+1; sock++)
        //         {
        //                 if(FD_ISSET(sock, &reads))
        //                 {
        //                         if(sock==serv_sock)     // connection request!
        //                         {
        //                                 adr_sz=sizeof(clnt_adr);
        //                                 clnt_sock=
        //                                         accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        //                                 FD_SET(clnt_sock, &reads);
        //                                 if(fd_max<clnt_sock)
        //                                         fd_max=clnt_sock;
        //                                 printf("connected client: %d \n", clnt_sock);
        //                         }
        //                         else    // read message!
        //                         {
        //                                 memset(buf, 0, BUF_SIZE);
        //                                 str_len=read(sock, buf, BUF_SIZE);
        //                                 if(str_len==0)    // close request!
        //                                 {
        //                                         FD_CLR(sock, &reads);
        //                                         close(sock);
        //                                         printf("closed client: %d \n", sock);
        //                                 }
        //                                 else
        //                                 {
        //                                         printf("client: %s \n", buf);
        //                                         write(sock, buf, str_len);    // echo!
        //                                 }
        //                         }
        //                 }
        //         }
        // }

        close(serv_sock);
        return 0;
}