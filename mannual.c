#include <stdio.h>
#include <string.h>

int main() {
    char command[22] = {0,}, option[22]={0,};  
    scanf("%s %s", &command, &option);
    
    if(!strcmp(command,"help")) {//if는 맞으면 1, strcmp는 맞으면 0
        if(!strcmp(option,"chat")){
            printf("다른 유저와 채팅을 시도합니다.\n");
            printf("chat [닉네임]\n");
            printf("채팅 연결은 유저의 온라인 상태 여부에 따라 달라집니다.\n");
            printf("유저의 상태가 온라인이면 채팅이 시작됩니다.\n");
            printf("채팅이 시작되면 메시지를 입력하여 대화할 수 있습니다.\n");
            printf("유저의 상태가 오프라인이면 연결되지 않습니다.\n");
        }
        
        
        if(!strcmp(option,"exit")){
            printf("서버와의 연결을 종료합니다.\n");
        }
        
        if(!strcmp(option,"login")){
            printf("유저를 로그인 시킵니다.\n");
            printf("login [닉네임]\n");
            printf("로그인합니다.\n");
        }
        
        if(!strcmp(option,"logout")){
            printf("유저를 로그아웃 시킵니다.\n");
            printf("logout\n");
        }

        if(!strcmp(option,"")){
            printf("특정 명령어에 대한 자세한 내용이 필요하면 help [명령어 이름]을 입력하세요.\n");
            printf("명령어 종류는 아래와 같습니다.\n");
            printf("chat	다른 유저와 채팅을 하고 싶을 때 사용합니다.\n");
            printf("exit	서버와의 연결을 종료합니다.\n");
            printf("login	서버에 로그인합니다.\n");
            printf("logout	서버에 로그아웃 합니다.\n");
        }

        else
            printf("알아듣게 치세요. ^^;;\n");
    }
    
}
