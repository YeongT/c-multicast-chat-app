#pragma once

#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_CLIENT 100
#define MAX_GROUP 10
#define MAX_GROUP_MEMBER 10
#define MAX_BUF (SIZE_STATUS_CODE + SIZE_OPTION + SIZE_MESSAGE)

#define SIZE_CMD_CODE sizeof(int)
#define SIZE_STATUS_CODE sizeof(int)
#define SIZE_OPTION 30
#define SIZE_IP 15
#define SIZE_PORT 4
#define SIZE_MESSAGE 4096

#define COMMAND_RESULT 1000
#define COMMAND_LOGIN 1001
#define COMMAND_USERS 1002
#define COMMAND_CHECK 1003
#define COMMAND_CHAT 1004
#define COMMAND_MULTICAST 1005
#define COMMAND_GROUP_CHECK_JOIN 1006
#define COMMAND_GROUP_CHAT 1007
#define COMMAND_GROUP_JOIN 1008
#define COMMAND_GROUP_QUIT 1009
#define COMMAND_GROUP_MAKE 1010

#define RESPONSE_LOGIN_SUCCESS 2000
#define RESPONSE_CHECK_ONLINE 2100
#define RESPONSE_GROUP_JOINED 2200
#define RESPONSE_INFORMATION 3000
#define RESPONSE_GROUP_MESSAGE 4000
#define RESPONSE_LOGIN_FAILED 5000
#define RESPONSE_CHECK_OFFLINE 5100
#define RESPONSE_GROUP_NOT_JOINED 5200
#define RESPONSE_ERROR -1

#define CLIENT_STATUS_WAIT 0
#define CLIENT_STATUS_CHAT 1
#define CLIENT_STATUS_GROUP 2

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
