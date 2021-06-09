#include <stdio.h>
#include <stdlib.h>

//import define.h and multicast.h
#include "../define.h"
#include "../utils.h"
#include "../group.h"
#include "../multicast.h"

//# define userSocketObject
typedef struct userFrame
{
    int sock;
    char nickname[SIZE_OPTION];
} userSocketObject;

//# return userObject using userName
userSocketObject* getUserObjectByNickname(char *userName, userSocketObject *userList)
{
    for (int i = 0; i < MAX_CLIENT; i++)
        if (strcmp(userList[i].nickname, userName) == 0)
            return &userList[i];
    return NULL;
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
    write(sock, serverMsgContainer, MAX_BUF);
}

//# sendChatMessageToClient
void sendChatMessageToClient(int sock, char *chatClient, char *chatMessage, char *serverMsgContainer, bool sendInstant)
{
    memset(serverMsgContainer, 0, SIZE_MESSAGE);

    chatObject toMessage;
    strcpy(toMessage.client, chatClient);
    strcpy(toMessage.message, chatMessage);

    dataObject sendObject;
    convertChatObjectToDataObject(&toMessage, &sendObject);
    convertDataObjectToDataObjectString(&sendObject, serverMsgContainer);
    if (sendInstant == true)
        write(sock, serverMsgContainer, MAX_BUF);
}

//# split command process
void serverCommandCenter(dataObject *income, char *serverComment, char *sendMsg, userSocketObject *userList, int userID)
{
    if (income->cmdCode == COMMAND_LOGIN)
    {
        optionObject loginReq;
        convertOptionStringToOptionObject(income->body, &loginReq);
        userSocketObject *target = getUserObjectByNickname(loginReq.argument, userList);
        if (target == NULL)
        {
            strcpy(userList[userID].nickname, loginReq.argument);
            sprintf(serverComment, "Welcome user '%s'! Have a nice day!", userList[userID].nickname);
            respondToClient(userList[userID].sock, RESPONSE_LOGIN_SUCCESS, serverComment, sendMsg);
            fprintf(stdout, "[System Login] user '%s' signed in to system.\n", userList[userID].nickname);
            return;
        }
        sprintf(serverComment, "nickname '%s' is already in use", loginReq.argument);
        respondToClient(userList[userID].sock, RESPONSE_LOGIN_FAILED, serverComment, sendMsg);
    }
    else if (income->cmdCode == COMMAND_USERS)
    {
        optionObject userReq;
        convertOptionStringToOptionObject(income->body, &userReq);

        char userListString[MAX_CLIENT * SIZE_MESSAGE];
        memset(userListString, 0, MAX_CLIENT * SIZE_OPTION);
        if (strcmp(userReq.argument, "global") == 0)
        {
            sprintf(userListString, "Below users are online in group '%s'", userReq.argument);
            for (int user = 0; user < MAX_CLIENT; user++) 
                if (userList[user].sock != -1 && strcmp(userList[user].nickname, "") != 0)
                {
                    strcat(userListString,"\n  > ");
                    strcat(userListString, userList[user].nickname);
                }
            respondToClient(userList[userID].sock, RESPONSE_INFORMATION, userListString, sendMsg);
        }
    }
    else if (income->cmdCode == COMMAND_CHECK)
    {
        optionObject vitalCheck;
        convertOptionStringToOptionObject(income->body, &vitalCheck);
        userSocketObject *target = getUserObjectByNickname(vitalCheck.argument, userList);
        if (target != NULL)
        {
            sprintf(serverComment, "[INFO] nickname '%s' is trying to chat with you", userList[userID].nickname);
            respondToClient(target->sock, RESPONSE_INFORMATION, serverComment, sendMsg);
        }
        sprintf(serverComment, "nickname '%s' is %s", vitalCheck.argument, target != NULL ? "online" : "not online");
        respondToClient(userList[userID].sock, target != NULL ? RESPONSE_CHECK_ONLINE : RESPONSE_CHECK_OFFLINE, serverComment, sendMsg);
    }
    else if (income->cmdCode == COMMAND_CHAT)
    {
        chatObject fromMessage;
        convertChatStringToChatObject(income->body, &fromMessage);
        userSocketObject *target = getUserObjectByNickname(fromMessage.client, userList);
        if (target == NULL)
        {
            sprintf(serverComment, "nickname '%s' is not online", fromMessage.client);
            respondToClient(userList[userID].sock, RESPONSE_CHECK_OFFLINE, serverComment, sendMsg);
            return;
        }
        sendChatMessageToClient(target->sock, userList[userID].nickname, fromMessage.message, sendMsg, true);
    }
    else
        respondToClient(userList[userID].sock, RESPONSE_ERROR, "Unknown Command Inputed", sendMsg);
}

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd, sockfd;
    int state = 0, client_len, maxfd;
    int multi_sock;

    struct sockaddr_in clientaddr, serveraddr, multiaddr;
    fd_set readfds, otherfds, allfds;
    connectObject connectInfo;

    //# argument option
    if (argc != 3)
    {
        fprintf(stdout, "Usage : %s <multicast ip> <multicast port>\n", argv[0]);
        exit(1);
    }

    //# set udpServerConnectionInfo
    initializeServerMultiSock(&multi_sock, &multiaddr, &connectInfo, argv);

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
    serveraddr.sin_port = htons(connectInfo.port);
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

    fprintf(stdout, "\nUDP multiCast Server Starting ... on %s:%d", argv[1], atoi(argv[2]));
    fprintf(stdout, "\nTCP iomux multiplexing Server Starting ... on %s:%d\n\n", connectInfo.ip, connectInfo.port);

    char connectMessage[MAX_BUF];
    memset(connectMessage, 0, MAX_BUF);

    dataObject broadCast;
    convertConnectObjectToDataObject(&connectInfo, &broadCast);
    convertDataObjectToDataObjectString(&broadCast, connectMessage);

    userSocketObject users[MAX_CLIENT];
    for (int i = 0; i < MAX_CLIENT; i++)
        users[i].sock = -1;

    //# run server task forever
    time_t userTimer, multiTimer, now;
    time(&userTimer);
    time(&multiTimer);
    fprintf(stdout, "[System] start multiCasting iomux multiplexing server connect Info[%s:%d]\n", connectInfo.ip, connectInfo.port);
    while (1)
    {
        struct timeval timer = {1, 0};
        timer.tv_sec = 1;
        time(&now);

        allfds = readfds;
        state = select(maxfd + 1, &allfds, (fd_set *)0, (fd_set *)0, &timer);

        //# MultiCast Server : Deploy tcp_Iomux_Server_ConnectInfo
        if (difftime(now, multiTimer) >= 1)
        {
            sendto(multi_sock, connectMessage, MAX_BUF, 0, (struct sockaddr *)&multiaddr, sizeof(multiaddr));
            time(&multiTimer);
        }

        //# Show User List
        if (difftime(now, userTimer) >= 30)
        {
            fprintf(stdout, "\n-------- User List --------\n");
            for (int i = 0; i < MAX_CLIENT; i++)
                if (users[i].sock > 0)
                    fprintf(stdout, "   [User] : %s\n", users[i].nickname);
            fprintf(stdout, "--------    END    --------\n\n");
            time(&userTimer);
        }

        //# Server Socket - accept from client and save to userObject
        if (FD_ISSET(server_sockfd, &allfds))
        {

            //# return error message to client
            int count = 0;
            for (int i = 0; i < MAX_CLIENT; i++)
                if (users[i].sock != -1)
                    count++;
            if (count == MAX_CLIENT)
            {
                // => assign to other developer
                perror("too many clients");
            }

            client_len = sizeof(clientaddr);
            client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
            // fprintf(stdout, "\n[System] connection from (%s , %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

            //# save user socket
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (users[i].sock == -1)
                {
                    users[i].sock = client_sockfd;
                    fprintf(stdout, "[Socket] new client! socket opened to array[%d], clientFD=%d\n", i, users[i].sock);
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
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            if (users[i].sock == -1)
                continue;

            //# if users[i].sock has any state-value
            char recvMsg[MAX_BUF], sendMsg[MAX_BUF];
            if (FD_ISSET(users[i].sock, &allfds))
            {
                //# check whether socket is valid
                memset(recvMsg, 0, sizeof(recvMsg));
                if (read(users[i].sock, recvMsg, sizeof(recvMsg)) > 0)
                {
                    char serverComment[SIZE_MESSAGE];
                    memset(serverComment, 0, sizeof(serverComment));

                    dataObject income;
                    convertDataObjectStringToDataObject(recvMsg, &income);
                    serverCommandCenter(&income, serverComment, sendMsg, users, i);
                }
                else
                {
                    //# remove client socket from array
                    fprintf(stdout, "[System Logout] user '%s' signed out to system.\n", users[i].nickname);
                    fprintf(stdout, "[Socket] socket closed in array[%d], clientFD=%d\n\n", i, sockfd);
                    close(users[i].sock);

                    FD_CLR(users[i].sock, &readfds);
                    users[i].sock = -1;
                    memset(users[i].nickname, 0, SIZE_OPTION);
                }

                if (--state <= 0)
                    break;
            }
        }
    }
    close(multi_sock);
}