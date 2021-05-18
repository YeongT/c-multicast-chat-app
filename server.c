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
typedef struct  {
   int sock;
   char nickname[SIZE_OPTION];
} userSocketObject;

//# return valid client number
int getClientNumber(userSocketObject* userList) {
    int count = 0;
    for (int i=0; i<MAX_CLIENT; i++) if (userList[i].sock != -1) count++;
    return count;
}

//# return sock using userName
int getUserSock(char* userName, userSocketObject* userList) {
    int sock = -1;
    for (int i=0; i<MAX_CLIENT; i++) if(strcmp(userList[i].nickname, userName) == 0) sock = userList[i].sock;
    return sock;
}

//# split command-center-module from main
void commandCenter(int sock, int command, char* optionText) {
    switch (command) {
        case COMMAND_LOGIN:
            break;
        case COMMAND_CHECK:
            break;
        case COMMAND_CHAT:
            break;
        default:
            
            break;
    }
}

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd, sockfd;
    int state=0, client_len, maxfd;

    struct sockaddr_in clientaddr, serveraddr;
    struct timeval tv;
    fd_set readfds, otherfds, allfds;

    char buf[255];

    //# argument option
        if(argc!=2) {
            printf("Usage : %s <port>\n", argv[0]);
            exit(1);
        }

    //# server socket error handle
        if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket error ");
            exit(0);
        }

    //# open server tcp socket (iomux_select server) 
        memset(&serveraddr, 0x00, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(atoi(argv[1]));
        state = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (state == -1) {
            perror("bind error ");
            exit(0);
        }

        state = listen(server_sockfd, 5);
        if (state == -1) {
            perror("listen error ");
            exit(0);
        }

    //# iomux_select server setup
        client_sockfd = server_sockfd;
        maxfd = server_sockfd;

        FD_ZERO(&readfds);
        FD_SET(server_sockfd, &readfds);

        printf("\nTCP Server Starting ... on %d\n\n", atoi(argv[1]));
        fflush(stdout);

    //# run server task forever
        userSocketObject users[MAX_CLIENT];
        for (int i=0; i<MAX_CLIENT; i++) users[i].sock = -1;
        while(1)
        {
            allfds = readfds;
            state = select(maxfd + 1 , &allfds, NULL, NULL, NULL);

            //# Server Socket - accept from client and save to userObject
            if (FD_ISSET(server_sockfd, &allfds)) {
                //# return error message to client
                if (getClientNumber(users) == MAX_CLIENT) {
                    // => assign to other developer
                    perror("too many clients\n");
                }
                
                client_len = sizeof(clientaddr);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
                printf("[System] connection from (%s , %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                //# save user socket
                for (int i = 0; i < MAX_CLIENT; i++)
                {
                    if (users[i].sock == -1 ) {
                        users[i].sock = client_sockfd;
                        printf("[System] new client! socket opened to array[%d], clientFD=%d\n", i, client_sockfd);
                        break;
                    }
                }

                //# for iomux_select server
                FD_SET(client_sockfd,&readfds);
                if (client_sockfd > maxfd) maxfd = client_sockfd;
                if (--state <= 0) continue;
            }

            //# check whether any message have been received
            for (int i = 0; i <= getClientNumber(users); i++)
            {
                if ((sockfd = users[i].sock) == -1) continue;

                //# if users[i].sock has any state-value
                if (FD_ISSET(sockfd, &allfds))
                {
                    memset(buf, 0, SIZE_MESSAGE);

                    //# check whether socket is valid
                    if (read(sockfd, buf, SIZE_MESSAGE) > 0){
                        printf("[CLIENT %d]RECV: %s\n", i, buf);
                        write(sockfd, buf, SIZE_MESSAGE);
                    } else {
                        //# remove client socket from array
                        printf("\n===================================\n");
                        printf("[Socket] socket closed in array[%d], clientFD=%d\n", i, sockfd);
                        close(sockfd);
                        
                        FD_CLR(sockfd, &readfds);
                        users[i].sock = -1;
                        memset(users[i].nickname, 0, SIZE_MESSAGE);
                    }

                    if (--state <= 0) break;
                }
            }

        }
}