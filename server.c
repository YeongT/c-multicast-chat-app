#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

// #include <WinSock2.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

//# share profile with server
#include "define.h"

//# define userSocketObject
typedef struct
{
    int sock;
    char nickname[SIZE_OPTION];
} userSocketObject;

//# return valid client number
int getClientNumber(userSocketObject *userList)
{
    int count = 0;
    for (int i = 0; i < MAX_CLIENT; i++)
        if (userList[i].sock != -1)
            count++;
    return count;
}

//# return sock using userName
int getUserSockByNickname(char *userName, userSocketObject *userList)
{
    int sock = -1;
    for (int i = 0; i < MAX_CLIENT; i++)
        if (strcmp(userList[i].nickname, userName) == 0)
            sock = userList[i].sock;
    return sock;
}

//# sendResultToClient
void respondToClient(int sock, int status, char *message, char *serverMsgContainer)
{
    memset(serverMsgContainer, 0, sizeof(serverMsgContainer));

    resultObject response;
    response.status = status;
    memset(response.message, 0, SIZE_MESSAGE);
    strcpy(response.message, message);

    dataObject respond;
    convertResultObjectToDataObject(&response, &respond);
    convertDataObjectToDataObjectString(&respond, serverMsgContainer);
    write(sock, serverMsgContainer, MAX_BUF); //여기서 문제 발생 or read()
}

//# split command process
void commandCenter(dataObject *income, userSocketObject *users, char *serverComment, char *sendMsg, int i) 
{   
    if (income->cmdCode == COMMAND_LOGIN)
    {
        optionObject loginReq;
        convertOptionStringToOptionObject(income->body, &loginReq);
        if (getUserSockByNickname(loginReq.argument, users) == -1)
        {
            strcpy(users[i].nickname, loginReq.argument);
            sprintf(serverComment, "Welcome user '%s'! Have a nice day!", users[i].nickname);
            respondToClient(users[i].sock, RESPONSE_LOGIN_SUCCESS, serverComment, sendMsg);
            fprintf(stdout, "[System Login] user '%s' signed in to system.\n", users[i].nickname);
        }
        else
        {
            sprintf(serverComment, "nickname '%s' is already in use", loginReq.argument);
            respondToClient(users[i].sock, RESPONSE_LOGIN_FAILED, serverComment, sendMsg);
        }
    }
    else
        respondToClient(users[i].sock, RESPONSE_ERROR, "UnAcceptable Command Inputed", sendMsg);
}

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd, sockfd;
    int state = 0, client_len, maxfd;

    struct sockaddr_in clientaddr, serveraddr;
    fd_set readfds, otherfds, allfds;
    struct timeval tv;
    userSocketObject users[MAX_CLIENT];

    //# argument option
    if (argc != 2)
    {
        fprintf(stdout, "Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    //# server socket error handle
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error ");
        exit(0);
    }

    //# open server tcp socket (iomux_select server)
    memset(&serveraddr, 0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));
    state = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (state == -1)
    {
        perror("bind error ");
        exit(0);
    }

    state = listen(server_sockfd, 5);
    if (state == -1)
    {
        perror("listen error ");
        exit(0);
    }

    //# iomux_select server setup
    client_sockfd = server_sockfd;
    maxfd = server_sockfd;

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);

    fprintf(stdout, "\nTCP Server Starting ... on %d\n", atoi(argv[1]));

    //# run server task forever
    for (int i = 0; i < MAX_CLIENT; i++)
        users[i].sock = -1;
    while (1)
    {
        allfds = readfds;
        state = select(maxfd + 1, &allfds, NULL, NULL, NULL);

        //# Server Socket - accept from client and save to userObject
        if (FD_ISSET(server_sockfd, &allfds))
        {
            //# return error message to client
            if (getClientNumber(users) == MAX_CLIENT)
            {
                // => assign to other developer
                perror("too many clients\n");
            }

            client_len = sizeof(clientaddr);
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
            fprintf(stdout, "\n[System] connection from (%s , %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

            //# save user socket
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (users[i].sock == -1)
                {
                    users[i].sock = client_sockfd;
                    fprintf(stdout, "[Socket] new client! socket opened to array[%d], clientFD=%d\n", i, client_sockfd);
                    break;
                }
            }

            //# for iomux_select server
            FD_SET(client_sockfd, &readfds);
            if (client_sockfd > maxfd)
                maxfd = client_sockfd;
            if (--state <= 0)
                continue;
        }

        //# check whether any message have been received
        for (int i = 0; i <= getClientNumber(users); i++)
        {
            if ((sockfd = users[i].sock) == -1)
                continue;

            //# if users[i].sock has any state-value
            char recvMsg[MAX_BUF], sendMsg[MAX_BUF];
            if (FD_ISSET(sockfd, &allfds))
            {
                //# check whether socket is valid
                memset(recvMsg, 0, sizeof(recvMsg));
                if (read(sockfd, recvMsg, sizeof(recvMsg)) > 0)
                {
                    char serverComment[SIZE_MESSAGE];
                    memset(serverComment, 0, sizeof(serverComment));

                    dataObject income;;
                    convertDataObjectStringToDataObject(recvMsg, &income);
                    commandCenter(&income, users, serverComment, sendMsg, i);
                }
                else
                {
                    //# remove client socket from array
                    fprintf(stdout, "[System Logout] user '%s' signed out to system.\n", users[i].nickname);
                    fprintf(stdout, "[Socket] socket closed in array[%d], clientFD=%d\n", i, sockfd);
                    close(sockfd);

                    FD_CLR(sockfd, &readfds);
                    users[i].sock = -1;
                    memset(users[i].nickname, 0, SIZE_MESSAGE);
                }

                if (--state <= 0)
                    break;
            }
        }
    }
}