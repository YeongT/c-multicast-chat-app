#include <string.h>
#include <stdbool.h>

#define MAX_CLIENT 100
#define MAX_BUF (SIZE_STATUS_CODE + SIZE_OPTION + SIZE_MESSAGE)

#define SIZE_CMD_CODE sizeof(int)
#define SIZE_STATUS_CODE sizeof(int)
#define SIZE_OPTION 30
#define SIZE_IP 15
#define SIZE_PORT 4
#define SIZE_MESSAGE 1024

#define COMMAND_RESULT 1000
#define COMMAND_LOGIN 1001
#define COMMAND_LOGOUT 1002
#define COMMAND_CHECK 1003
#define COMMAND_CHAT 1004
#define COMMAND_MULTICAST 1005
#define COMMAND_VITAL 1006

#define RESPONSE_LOGIN_SUCCESS 2000
#define RESPONSE_LOGIN_FAILED 5000
#define RESPONSE_CHECK_ONLINE 2001
#define RESPONSE_CHECK_OFFLINE 5001
#define RESPONSE_ERROR -1

void convertIntegerToAscii(int integer, char *string)
{
    sprintf(string, "%d", integer);
    return;
}

int removeNullCharacterFromString(char *origin, char *target)
{
    int current = 0;
    for (int i = 0; i < sizeof(origin); i++)
    {
        if (origin[i] != '\0')
        {
            strcpy(&target[current], &origin[i]);
            current++;
        }
    }
    return sizeof(origin) - current;
}

//extract cmdCode text from string to return casted integer
int getStatusCodeFromBodyString(char *originBodyString)
{
    char statusCode[SIZE_STATUS_CODE];
    memset(statusCode, 0, SIZE_STATUS_CODE);
    memcpy(statusCode, originBodyString, SIZE_STATUS_CODE);
    return atoi(statusCode);
}

//copy option string from bodyString to target string pointer
void copyOptionFromBodyString(char *originBodyString, char *targetOptionString)
{
    memset(targetOptionString, 0, SIZE_OPTION);
    memcpy(targetOptionString, originBodyString, SIZE_OPTION);
}

//copy message string from bodyString to target string pointer decided by command code
void copyMessageFromBodyString(int cmdCode, char *originBodyString, char *targetMessageString)
{
    memset(targetMessageString, 0, SIZE_OPTION);
    memcpy(targetMessageString, &originBodyString[cmdCode == COMMAND_RESULT ? SIZE_STATUS_CODE : SIZE_OPTION], SIZE_MESSAGE);
}

typedef struct dataFrame
{
    int cmdCode;
    char body[MAX_BUF - SIZE_CMD_CODE];
} dataObject;

void convertDataObjectStringToDataObject(char *originDataObjectString, dataObject *targetDataObject)
{
    char cmdCode[SIZE_CMD_CODE];
    memset(cmdCode, 0, SIZE_CMD_CODE);
    memcpy(cmdCode, originDataObjectString, SIZE_CMD_CODE);
    targetDataObject->cmdCode = atoi(cmdCode);
    memcpy(targetDataObject->body, &originDataObjectString[SIZE_CMD_CODE], sizeof(targetDataObject->body));
}

void convertDataObjectToDataObjectString(dataObject *originDataObject, char *targetDataObjectString)
{
    char cmdCodeString[SIZE_CMD_CODE];
    memset(cmdCodeString, 0, SIZE_CMD_CODE);
    memset(targetDataObjectString, 0, sizeof(targetDataObjectString));
    convertIntegerToAscii(originDataObject->cmdCode, cmdCodeString);
    strcpy(targetDataObjectString, cmdCodeString);
    memcpy(&targetDataObjectString[SIZE_CMD_CODE], originDataObject->body, sizeof(originDataObject->body));
}

typedef struct resultFrame
{
    int status;
    char message[SIZE_MESSAGE];
} resultObject;

void convertResultStringToResultObject(char *originResultString, resultObject *targetResultObject)
{
    targetResultObject->status = getStatusCodeFromBodyString(originResultString);
    copyMessageFromBodyString(COMMAND_RESULT, originResultString, targetResultObject->message);
}

void convertResultObjectToDataObject(resultObject *originResultObject, dataObject *targetDataObject)
{
    char statusCodeString[SIZE_STATUS_CODE];
    memset(statusCodeString, 0, SIZE_STATUS_CODE);
    memset(targetDataObject->body, 0, SIZE_STATUS_CODE + SIZE_MESSAGE);

    targetDataObject->cmdCode = COMMAND_RESULT;
    convertIntegerToAscii(originResultObject->status, statusCodeString);
    strcpy(targetDataObject->body, statusCodeString);
    memcpy(&targetDataObject->body[SIZE_STATUS_CODE], originResultObject->message, SIZE_MESSAGE);
}

typedef struct optionFrame
{
    char argument[SIZE_OPTION];
} optionObject;

void convertOptionStringToOptionObject(char *originOptionString, optionObject *targetOptionObject)
{
    copyOptionFromBodyString(originOptionString, targetOptionObject->argument);
}

void convertOptionObjectToDataObject(int cmdCode, optionObject *originOptionObject, dataObject *targetDataObject)
{
    targetDataObject->cmdCode = cmdCode;
    memset(targetDataObject->body, 0, SIZE_OPTION);
    memcpy(targetDataObject->body, originOptionObject->argument, SIZE_OPTION);
}

typedef struct chatFrame
{
    char client[SIZE_OPTION];
    char message[SIZE_MESSAGE];
} chatObject;

void convertChatStringToChatObject(char *originChatString, chatObject *targetChatObject)
{
    copyOptionFromBodyString(originChatString, targetChatObject->client);
    copyMessageFromBodyString(COMMAND_CHAT, originChatString, targetChatObject->message);
}

void convertChatObjectToDataObject(chatObject *originChatObject, dataObject *targetDataObject)
{
    memset(targetDataObject->body, 0, SIZE_OPTION + SIZE_MESSAGE);
    memcpy(targetDataObject->body, originChatObject->client, SIZE_OPTION);
    memcpy(&targetDataObject->body[SIZE_OPTION], originChatObject->message, SIZE_MESSAGE);
}

typedef struct connectFrame
{
    int port;
    char ip[15];
} connectObject;

void convertConnectStringToConnectObject(char *originConnectString, connectObject *targetConnectObject)
{
    char portText[4];
    memset(targetConnectObject->ip, 0, SIZE_IP);
    memcpy(targetConnectObject->ip, originConnectString, 15);
    memset(portText, 0, SIZE_PORT);
    memcpy(portText, &originConnectString[SIZE_IP], SIZE_PORT);
    targetConnectObject->port = atoi(portText);
}

void convertConnectObjectToDataObject(connectObject *originConnectObject, dataObject *targetDataObject)
{
    targetDataObject->cmdCode = COMMAND_MULTICAST;
    memset(targetDataObject->body, 0, SIZE_IP + SIZE_PORT);
    memcpy(targetDataObject->body, originConnectObject->ip, SIZE_IP);
    convertIntegerToAscii(originConnectObject->port, &targetDataObject->body[SIZE_IP]);
}