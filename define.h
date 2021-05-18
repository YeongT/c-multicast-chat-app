#include <string.h>

#define MAX_CLIENT 100

#define SIZE_CMD_CODE sizeof(int)
#define SIZE_OPTION 20
#define SIZE_MESSAGE 1024

#define COMMAND_LOGIN 1000
#define COMMAND_CHECK 1001
#define COMMAND_CHAT 1002

typedef struct {
    int status;
    char message[SIZE_MESSAGE];
} responseObject;

typedef struct {
    int command;
    char value[SIZE_OPTION];
} commandObject;

typedef struct {
    int command;
    char client[SIZE_OPTION];
    char message[SIZE_MESSAGE];
} messageObject;


void intToAscii(int integer, char* stringPointer) {
   sprintf(stringPointer, "%d", integer);
   return;
}

//# convert extracted cmdCode text in string to target, return residual text's pointer
char* extractCmdCodeFromString(char* originStringPointer, int* targetIntegerPointer) {
    char code[SIZE_CMD_CODE];
    memset(code, 0, SIZE_CMD_CODE);
    memcpy(code, originStringPointer, SIZE_CMD_CODE);
    *targetIntegerPointer = atoi(code);
    string = string[SIZE_CMD_CODE];
    return string;
}

//# extract option from string to target
void* extractOptionFromString(char* cmdCodeRemovedStringPointer, char* targetOptionStringPointer) {
    memset(targetOptionStringPointer, 0, SIZE_OPTION);
    memcpy(targetOptionStringPointer, cmdCodeRemovedStringPointer, SIZE_OPTION);
}

//# extract message from string to target
void extractMessageFromString(char* cmdCodeRemovedStringPointer, char* targetMessageStringPointer) {
    memset(targetMessageStringPointer, 0, SIZE_MESSAGE);
    memcpy(targetMessageStringPointer, &cmdCodeRemovedStringPointer[SIZE_OPTION], SIZE_MESSAGE);
}

//# save combined string to targetString, (cmdCodeString and valueString)
void* generateCommandString(char* cmdCodeStringPointer, char* valueStringPointer, char* intergatedStringPointer) {
    
}

//# save combined string to targetString, (cmdCodeString, clientString, and messageString)
void* generateCommandString(char* cmdCodeStringPointer, char* clientStringPointer, char* messageStringPointer, char* intergatedStringPointer) {
    
}