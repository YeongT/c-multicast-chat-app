#include <stdio.h>
#include <stdlib.h>

//import define.h and multicast.h
#include "../define.h"
#include "../utils.h"
#include "../methods.h"
#include "../multicast.h"

//# FD_ISSET command Center
void serverCommandCenter(dataObject *income, char *serverComment, char *sendMsg, userSocketObject *userList, userGroupObject *groupList, int userID)
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
            joinUserGroup(&groupList[0], &userList[userID]);
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
        memset(userListString, 0, MAX_CLIENT * SIZE_MESSAGE);

        userGroupObject *target = getGroupObjectByGroupName(groupList, userReq.argument);
        if (target == NULL)
        {
            sprintf(serverComment, "group '%s' is not exist", userReq.argument);
            respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            return;
        }

        sprintf(userListString, "Below users are online in group '%s'", userReq.argument);
        for (int user = 0; user < target->maxMember; user++)
        {
            if (target->members[user] == NULL)
                continue;
            if (target->members[user]->sock != -1 && strcmp(target->members[user]->nickname, "") != 0)
            {
                strcat(userListString, "\n  > ");
                strcat(userListString, target->members[user]->nickname);
            }
        }
        respondToClient(userList[userID].sock, RESPONSE_INFORMATION, userListString, sendMsg);
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
    else if (income->cmdCode == COMMAND_GROUP_MAKE)
    {
        optionObject groupReq;
        convertOptionStringToOptionObject(income->body, &groupReq);
        userGroupObject *target = getGroupObjectByGroupName(groupList, groupReq.argument);
        if (target == NULL)
        {
            bool result = generateGroupObject(groupList, MAX_GROUP_MEMBER, groupReq.argument);
            if (result)
            {
                sprintf(serverComment, "group '%s' has been created", groupReq.argument);
                respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
                return;
            }
            sprintf(serverComment, "[ERROR] Maximum number of groups exceeded.");
            respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            return;
        }
        sprintf(serverComment, "group '%s' is already exist", groupReq.argument);
        respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
    }
    else if (income->cmdCode == COMMAND_GROUP_CHECK_JOIN || income->cmdCode == COMMAND_GROUP_JOIN || income->cmdCode == COMMAND_GROUP_QUIT)
    {
        optionObject groupReq;
        convertOptionStringToOptionObject(income->body, &groupReq);
        userGroupObject *target = getGroupObjectByGroupName(groupList, groupReq.argument);
        if (target == NULL)
        {
            sprintf(serverComment, "group '%s' is not exist", groupReq.argument);
            respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            return;
        }

        int memberNum = getUserIdFromGroupByUserSock(target, &userList[userID]);
        switch (income->cmdCode)
        {
        case COMMAND_GROUP_CHECK_JOIN:
            sprintf(serverComment, "nickname '%s' %s group '%s'", userList[userID].nickname, memberNum != -1 ? "joined in" : "not joined in", groupReq.argument);
            respondToClient(userList[userID].sock, memberNum != -1 ? RESPONSE_GROUP_JOINED : RESPONSE_GROUP_NOT_JOINED, serverComment, sendMsg);
            break;
        case COMMAND_GROUP_JOIN:
            if (memberNum != -1)
            {
                sprintf(serverComment, "nickname '%s' is already in group '%s'", userList[userID].nickname, groupReq.argument);
                respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            }
            else
            {
                sprintf(serverComment, "nickname '%s' joined group '%s'!", userList[userID].nickname, groupReq.argument);
                broadCastToGroup(target, &userList[userID], serverComment, sendMsg);
                joinUserGroup(target, &userList[userID]);
                sprintf(serverComment, "join group '%s' succeed.", groupReq.argument);
                respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            }
            break;
        case COMMAND_GROUP_QUIT:
            if (memberNum == -1)
            {
                sprintf(serverComment, "nickname '%s' is not in group '%s'", userList[userID].nickname, groupReq.argument);
                respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            }
            else
            {
                sprintf(serverComment, "nickname '%s' left group '%s'.", userList[userID].nickname, groupReq.argument);
                broadCastToGroup(target, &userList[userID], serverComment, sendMsg);
                target->members[memberNum] = NULL;
                sprintf(serverComment, "quit group '%s' succeed.", groupReq.argument);
                respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);

                //# remove group when nobody left.
                int userCount = 0;
                for (int user = 0; user < target->maxMember; user++)
                    if (target->members[user] != NULL)
                        userCount++;

                if (userCount == 0)
                {
                    target->maxMember = 0;
                    memset(target->name, 0, SIZE_OPTION);
                    free(target->members);
                    target->members = NULL;
                }
            }
            break;
        }
    }
    else if (income->cmdCode == COMMAND_GROUP_CHAT)
    {
        chatObject fromMessage;
        convertChatStringToChatObject(income->body, &fromMessage);
        userGroupObject *target = getGroupObjectByGroupName(groupList, fromMessage.client);
        if (target == NULL)
        {
            sprintf(serverComment, "group '%s' is not exist", fromMessage.client);
            respondToClient(userList[userID].sock, RESPONSE_INFORMATION, serverComment, sendMsg);
            return;
        }
        else
            broadCastToGroup(target, &userList[userID], fromMessage.message, sendMsg);
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

    //# generate user and group object for service
    userSocketObject users[MAX_CLIENT];
    for (int i = 0; i < MAX_CLIENT; i++)
        users[i].sock = -1;
    userGroupObject groups[MAX_GROUP];
    for (int i = 0; i < MAX_GROUP; i++)
        groups[i].members = NULL;
    generateGroupObject(groups, MAX_CLIENT, "global");

    //# run server task forever
    time_t multiTimer, logTimer, now;
    time(&logTimer);
    time(&multiTimer);
    fprintf(stdout, "[System] start multiCasting iomux multiplexing server connect Info[%s:%d]\n\n", connectInfo.ip, connectInfo.port);
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
            manageGroupMemorySystem(groups);
            sendto(multi_sock, connectMessage, MAX_BUF, 0, (struct sockaddr *)&multiaddr, sizeof(multiaddr));
            time(&multiTimer);
        }

        //# Show User,Group List
        if (difftime(now, logTimer) >= 20)
        {
            fprintf(stdout, "\n-------- Group List --------\n");
            for (int i = 0; i < MAX_GROUP; i++)
                if (groups[i].members != NULL)
                    fprintf(stdout, "   [Group] : %s\n", groups[i].name);
            fprintf(stdout, "--------    END    --------\n");
            fprintf(stdout, "-------- User List --------\n");
            for (int i = 0; i < MAX_CLIENT; i++)
                if (users[i].sock > 0)
                    fprintf(stdout, "   [User] : %s\n", users[i].nickname);
            fprintf(stdout, "--------    END    --------\n\n");
            time(&logTimer);
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
                    serverCommandCenter(&income, serverComment, sendMsg, users, groups, i);
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
    close(server_sockfd);
}