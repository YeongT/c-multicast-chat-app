#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include <WinSock2.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//# share profile with server
#include "define.h"

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char aborter[30];

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    char sendMsg[MAX_BUF], recvMsg[MAX_BUF];
    while (1)
    {
        optionObject login;
        memset(sendMsg, 0, sizeof(sendMsg));
        memset(recvMsg, 0, sizeof(recvMsg));

        // fputs("if you want to quit, press q or Q : ", stdout);
        fputs("Please input your nickname. If you want to quit, press q or Q : \n", stdout);
        memset(login.argument, 0, sizeof(login.argument));
        fgets(login.argument, sizeof(login.argument), stdin);
        login.argument[strlen(login.argument) - 1] = '\0';

        if (!strcmp(login.argument, "q") || !strcmp(login.argument, "Q"))
        {
            fputs("Pressed q", stdout);
            exit(1);
        }

        dataObject send, receive;
        convertOptionObjectToDataObject(COMMAND_LOGIN, &login, &send);
        convertDataObjectToDataObjectString(&send, sendMsg);
        write(sock, sendMsg, sizeof(sendMsg));

        int str_len = read(sock, recvMsg, MAX_BUF);
        if (str_len == -1)
            error_handling("read() error!");
        convertDataObjectStringToDataObject(recvMsg, &receive);
        resultObject result;
        convertResultStringToResultObject(receive.body, &result);

        printf("\n\nCommand Code: %d\nStatus: %d\nMessage from server: %s\n", receive.cmdCode, result.status, result.message);

        if(result.status == RESPONSE_LOGIN_SUCCESS){
            fprintf(stdout, "Logined\n");
            break;
        }else{
            fprintf(stdout, "Nickname Already Exists.\n");
        }

        fflush(0);
    }
    while (1)
    {
        
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
