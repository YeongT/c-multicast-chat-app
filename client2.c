#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include <WinSock2.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//# share profile with server
#include "define.h"

#define BUF_SIZE 1024

void error_handling(char *message);



void get_info_from_multicast(char si[], char sp[]){
    memcpy(si, "456.46.6.6", 25));
    memcpy(sp, "4777", 4);
    // strcpy(sp, "6426");
    // printf("\n%s %s", si, sp);
    return;
}


int main(int argc, char *argv[])
{

    char server_ip[25];
    char server_port[4];

    get_info_from_multicast(server_ip, server_port);

    printf("\n%s %s sdfsdf ", server_ip, server_port);
    return 0;




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

        if (result.status == RESPONSE_LOGIN_SUCCESS)
        {
            fprintf(stdout, "Logined\n");
            break;
        }
        else
        {
            fprintf(stdout, "Nickname Already Exists.\n");
        }

        fflush(0);
    }

    int clnt_as_serv_sock, serv_as_clnt_sock;
    struct sockaddr_in clnt_as_serv_adr, serv_as_clnt_adr;
    struct timeval timeout;
    fd_set reads;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, sock2;
    int on = 1;

    clnt_as_serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&clnt_as_serv_adr, 0, sizeof(clnt_as_serv_adr));
    clnt_as_serv_adr.sin_family = AF_INET;
    clnt_as_serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    clnt_as_serv_adr.sin_port = htons(atoi("6666"));

    setsockopt(clnt_as_serv_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));

    if (bind(clnt_as_serv_sock, (struct sockaddr *)&clnt_as_serv_adr, sizeof(clnt_as_serv_adr)) == -1)
    {
        printf("bind() error");
        exit(0);
    }
    if (listen(clnt_as_serv_sock, 5) == -1)
    {
        printf("listen() error");
        exit(0);
    }

    FD_ZERO(&reads);
    FD_SET(clnt_as_serv_sock, &reads);
    fd_max = clnt_as_serv_sock;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int STDIN = fileno(stdin);
    char buf[1024];
    while (1)
    {
        FD_SET(clnt_as_serv_sock, &reads);
        FD_SET(STDIN, &reads);
        if ((fd_num = select(fd_max + 1, &reads, 0, 0, &timeout)) == -1)
            break;

        if (fd_num == 0)
        {
            printf("Heartbeat to Server~~\n");

            char sendMsg[MAX_BUF], recvMsg[MAX_BUF];

            optionObject heartbeat;
            memset(heartbeat.argument, 0, sizeof(heartbeat.argument));
            strcpy(heartbeat.argument, "tprtm");

            memset(sendMsg, 0, sizeof(sendMsg));
            memset(recvMsg, 0, sizeof(recvMsg));

            dataObject send, receive;
            convertOptionObjectToDataObject(COMMAND_CHECK, &heartbeat, &send);
            convertDataObjectToDataObjectString(&send, sendMsg);

            write(sock, sendMsg, sizeof(sendMsg));

            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            continue;
        }
        for (sock2 = 0; sock2 < fd_max + 1; sock2++)
        {
            if (FD_ISSET(sock2, &reads))
            {

                if (sock2 == STDIN)
                {
                    memset(buf, 0, BUF_SIZE);
                    str_len = read(sock2, buf, BUF_SIZE);
                    if (str_len == 0) // close request!
                    {
                    }

                    printf("Input : %s\n", buf);

                    break;
                }
                else if (sock2 == clnt_as_serv_sock)
                { //conn request

                    adr_sz = sizeof(clnt_as_serv_adr);
                    serv_as_clnt_sock = accept(clnt_as_serv_sock, (struct sockaddr *)&clnt_as_serv_adr, &adr_sz);

                    FD_SET(serv_as_clnt_sock, &reads);
                    if (fd_max < serv_as_clnt_sock)
                        fd_max = serv_as_clnt_sock;
                    printf("connected server_as_client: %d \n", serv_as_clnt_sock);
                }
                else
                {

                    printf("client read %d\n", sock2);
                    fflush(0);
                    memset(buf, 0, BUF_SIZE);
                    str_len = read(sock2, buf, BUF_SIZE);
                    if (str_len == 0) // close request!
                    {
                        FD_CLR(sock2, &reads);
                        close(sock2);
                        printf("closed client: %d \n", sock2);
                    }
                    else
                    {
                        printf("client: %s \n", buf);
                        write(sock2, buf, str_len); // echo!
                    }
                }
            }
        }

        // for (sock2 = 0; sock2 < fd_max + 1; sock2++)
        // {
        //     if (FD_ISSET(sock2, &reads))
        //     {
        //         if (sock2 == clnt_as_serv_sock) // connection request!
        //         {
        //             adr_sz = sizeof(clnt_adr2);
        //             clnt_sock2 =
        //                 accept(clnt_as_serv_sock, (struct sockaddr *)&clnt_adr2, &adr_sz);
        //             FD_SET(clnt_sock2, &reads);
        //             if (fd_max < clnt_sock2)
        //                 fd_max = clnt_sock2;
        //             printf("connected client: %d \n", clnt_sock2);
        //         }
        //         else // read message!
        //         {
        //             memset(buf, 0, BUF_SIZE);
        //             str_len = read(sock2, buf, BUF_SIZE);
        //             if (str_len == 0) // close request!
        //             {
        //                 FD_CLR(sock2, &reads);
        //                 close(sock2);
        //                 printf("closed client: %d \n", sock2);
        //             }
        //             else
        //             {
        //                 printf("client: %s \n", buf);
        //                 write(sock2, buf, str_len); // echo!
        //             }
        //         }
        //     }
        // }
    }
    close(clnt_as_serv_sock);

    close(sock2);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
