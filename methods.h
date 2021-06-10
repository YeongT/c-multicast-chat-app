//import define.h for shared-environment
#include "define.h"
#include "utils.h"

//# define userSocketObject
typedef struct userFrame
{
    int sock;
    char nickname[SIZE_OPTION];
} userSocketObject;

//# define userGroupObject
typedef struct groupFrame
{
    int maxMember;
    char name[SIZE_OPTION];
    userSocketObject **members;
} userGroupObject;

//# return userObject using userName
userSocketObject *getUserObjectByNickname(char *userName, userSocketObject *userList)
{
    for (int i = 0; i < MAX_CLIENT; i++)
        if (strcmp(userList[i].nickname, userName) == 0)
            return &userList[i];
    return NULL;
}

userGroupObject *getGroupObjectByGroupName(userGroupObject *groupList, char *groupName)
{
    for (int i = 0; i < MAX_GROUP; i++)
        if (strcmp(groupList[i].name, groupName) == 0)
            return &groupList[i];
    return NULL;
}

//# sendMessageToClient
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
    convertChatObjectToDataObject(&toMessage, &sendObject, false);
    convertDataObjectToDataObjectString(&sendObject, serverMsgContainer);
    if (sendInstant == true)
        write(sock, serverMsgContainer, MAX_BUF);
}

//# return userNum when user joined in group 
int getUserIdFromGroupByUserSock(userGroupObject *targetGroup, userSocketObject *findUserObject) {
    if (targetGroup->members == NULL) return -1;
    for (int userNum=0; userNum<targetGroup->maxMember; userNum++) 
        if (targetGroup->members[userNum] != NULL) 
            if (!strcmp(targetGroup->members[userNum]->nickname, findUserObject->nickname)) 
                return userNum;
    return -1;
}

//# send message to all users in targetGroup
void broadCastToGroup(userGroupObject *targetGroup, userSocketObject *sender, char *chatMessage, char *serverMsgContainer)
{
    char *message = malloc(SIZE_MESSAGE);
    strcpy(message, chatMessage);
    sprintf(chatMessage, "[Message from '%s' in group '%s'] : %s", sender->nickname, targetGroup->name, message);
    for (int user = 0; user < targetGroup->maxMember; user++)
    {
        if (targetGroup->members[user] == NULL)
            continue;

        if (targetGroup->members[user]->sock != -1 && targetGroup->members[user]->sock != sender->sock)
            respondToClient(targetGroup->members[user]->sock, RESPONSE_GROUP_MESSAGE, chatMessage, serverMsgContainer);
    }
    free(message);
}

bool generateGroupObject(userGroupObject *groupList, int maxMember, char *groupName)
{
    for (int groupNum = 0; groupNum < MAX_GROUP; groupNum++)
        if (groupList[groupNum].members == NULL)
        {
            strcpy(groupList[groupNum].name, groupName);
            groupList[groupNum].maxMember = maxMember;
            groupList[groupNum].members = malloc(__SIZEOF_POINTER__ * maxMember);

            for (int memberNum = 0; memberNum < groupList[groupNum].maxMember; memberNum++)
                groupList[groupNum].members[memberNum] = NULL;
            return true;
        }
    return false;
}

bool joinUserGroup(userGroupObject *targetGroup, userSocketObject *executer)
{
    for (int user = 0; user < targetGroup->maxMember; user++)
        if (targetGroup->members[user] == NULL)
        {
            targetGroup->members[user] = executer;
            return true;
        }
    return false;
}

void manageGroupMemorySystem(userGroupObject *groupList)
{
    for (int groupNum = 1; groupNum < MAX_GROUP; groupNum++)
    {
        if (groupList[groupNum].members == NULL)
            continue;

        for (int userNum = 0; userNum < groupList[groupNum].maxMember; userNum++)
        {
            if (groupList[groupNum].members[userNum] == NULL)
                continue;
            if (groupList[groupNum].members[userNum]->sock == -1)
                groupList[groupNum].members[userNum] = NULL;
        }
    }
}