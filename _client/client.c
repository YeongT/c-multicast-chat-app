#include <stdio.h>
#include <stdlib.h>

//# import define.h and multicast.h
#include "../define.h"
#include "../utils.h"
#include "../multicast.h"

int chatStatus = CLIENT_STATUS_WAIT;
dataObject sendData, recvData;

void startClientIomuxServer(fd_set *reads, int *fd_num, int *fd_max)
{
	struct sockaddr_in serv_adr;
	int on = 1;

	int clientIomuxSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);

	srand(time(NULL));
	int clientPort = rand() % 2000 + 6000;
	serv_adr.sin_port = htons(clientPort);

	setsockopt(clientIomuxSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
	if (bind(clientIomuxSock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
	{
		fprintf(stdout, "bind() error");
		exit(0);
	}
	if (listen(clientIomuxSock, 5) == -1)
	{
		fprintf(stdout, "listen() error");
		exit(0);
	}

	FD_ZERO(reads);
	FD_SET(clientIomuxSock, reads);
	*fd_max = clientIomuxSock;

	fprintf(stdout, "\n[SYSTEM] client one-way TCP multiPlexing Server Starting ... on %d\n\n", clientPort);
}

void executeHelpCenter(int helpCode)
{
	/*
		all : 0
		chat : 1
		users : 2
		group : 3
	*/
	switch (helpCode)
	{
	case 0:
		fprintf(stdout, "\n [HELP] 특정 명령어에 대한 자세한 내용이 필요하면 help [명령어 이름]을 입력하세요.\n");
		fprintf(stdout, ">  명령어 종류는 아래와 같습니다.\n");
		fprintf(stdout, ">  chat	다른 유저와 채팅을 하고 싶을 때 사용합니다.\n");
		fprintf(stdout, ">	group	그룹 채팅을 하기 위해 사용합니다.\n");
		fprintf(stdout, ">	users	그룹의 사용자 목록을 보여줍니다.\n");
		fprintf(stdout, ">  logout	서버에 로그아웃 합니다.\n");
		break;
	case 1:
		fprintf(stdout, "\n\n [CHAT] 다른 유저와 채팅을 시도합니다.\n");
		fprintf(stdout, ">  chat [닉네임]\n");
		fprintf(stdout, ">  채팅 연결은 유저의 온라인 상태 여부에 따라 달라집니다.\n");
		fprintf(stdout, ">  유저의 상태가 온라인이면 채팅이 시작됩니다.\n");
		fprintf(stdout, ">  채팅이 시작되면 메시지를 입력하여 대화할 수 있습니다.\n");
		fprintf(stdout, ">  유저의 상태가 오프라인이면 연결되지 않습니다.\n");
		break;
	case 2:
		fprintf(stdout, "\n\n [USERS] 해당 그룹의 사용자 목록을 출력합니다.\n");
		fprintf(stdout, ">  users [그룹이름]\n");
		fprintf(stdout, ">  모든 사용자는 'global'이라는 그룹에 속해 있습니다.\n");
		break;
	case 3:
		fprintf(stdout, "\n\n [GROUP] 그룹 채팅에 관련된 명령어 입니다.\n");
		fprintf(stdout, ">	group make [그룹명]\n");
		fprintf(stdout, ">	입력한 그룹명의 그룹을 만듭니다. 다른 사용자가 참가하고, 채팅할 수 있습니다.\n\n");
		fprintf(stdout, ">	group join [그룹명]\n");
		fprintf(stdout, ">	입력한 그룹명의 그룹에 참가합니다. 참가한 그룹의 그룹원들끼리 채팅할 수 있습니다.\n\n");
		fprintf(stdout, ">	group quit [그룹명]\n");
		fprintf(stdout, ">	참가한 그룹 방에서 나갑니다.\n\n");
		fprintf(stdout, ">	group chat [그룹명]\n");
		fprintf(stdout, ">	참가한 그룹의 그룹원들과 채팅합니다. 입력한 메시지는 그룹에 속한 모든 사용자에게 전달됩니다.\n");
		break;
	}

	fprintf(stdout, "\n");
}

void clientCommandCenter(char *sendMsg, char *recvMsg, int tcpSock, chatObject *sendChat)
{
	if (chatStatus != CLIENT_STATUS_WAIT)
	{
		if (strcmp(recvMsg, "/exit") == 0)
		{
			chatStatus == CLIENT_STATUS_CHAT ? fprintf(stdout, "[System] chatRoom with '%s' closed.\n\n", sendChat->client) : fprintf(stdout, "[System] chatRoom with group '%s' closed.\n\n", sendChat->client);
			chatStatus = CLIENT_STATUS_WAIT;
			return;
		}

		memset(sendChat->message, 0, SIZE_MESSAGE);
		memcpy(sendChat->message, recvMsg, SIZE_MESSAGE);

		memset(sendMsg, 0, MAX_BUF);
		convertChatObjectToDataObject(sendChat, &sendData, chatStatus == CLIENT_STATUS_GROUP ? true : false);
		convertDataObjectToDataObjectString(&sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);
		return;
	}

	//# assign two dimension char array
	char **input;
	input = (char **)malloc(sizeof(char *) * 5);
	for (int i = 0; i < 5; i++)
	{
		input[i] = (char *)malloc(sizeof(char) * SIZE_OPTION);
		memset(input[i], 0, sizeof(char) * SIZE_OPTION);
	}

	//# divide string into array
	int columnCount = splitStringByCharacter(recvMsg, ' ', input);
	if (strcmp(input[0], "chat") == 0)
	{
		if (columnCount != 2)
		{
			executeHelpCenter(1);
			return;
		}

		optionObject check;
		memset(sendMsg, 0, MAX_BUF);

		memset(check.argument, 0, SIZE_OPTION);
		strcpy(check.argument, input[1]);

		convertOptionObjectToDataObject(COMMAND_CHECK, &check, &sendData);
		convertDataObjectToDataObjectString(&sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);

		memset(sendChat->client, 0, SIZE_OPTION);
		strcpy(sendChat->client, input[1]);
	}
	else if (strcmp(input[0], "users") == 0)
	{
		if (columnCount != 2)
		{
			executeHelpCenter(2);
			return;
		}
		optionObject userReq;
		memset(sendMsg, 0, MAX_BUF);

		memset(userReq.argument, 0, SIZE_OPTION);
		strcpy(userReq.argument, input[1]);

		convertOptionObjectToDataObject(COMMAND_USERS, &userReq, &sendData);
		convertDataObjectToDataObjectString(&sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);
	}
	else if (strcmp(input[0], "group") == 0)
	{
		if (columnCount != 3)
		{
			executeHelpCenter(3);
			return;
		}

		int cmdCode;
		if (strcmp(input[1], "chat") == 0)
			cmdCode = COMMAND_GROUP_CHECK_JOIN;
		else if (strcmp(input[1], "make") == 0)
			cmdCode = COMMAND_GROUP_MAKE;
		else if (strcmp(input[1], "join") == 0)
			cmdCode = COMMAND_GROUP_JOIN;
		else if (strcmp(input[1], "quit") == 0)
			cmdCode = COMMAND_GROUP_QUIT;
		else
		{
			executeHelpCenter(3);
			return;
		}

		optionObject groupReq;
		memset(sendMsg, 0, MAX_BUF);

		memset(groupReq.argument, 0, SIZE_OPTION);
		strcpy(groupReq.argument, input[2]);

		convertOptionObjectToDataObject(cmdCode, &groupReq, &sendData);
		convertDataObjectToDataObjectString(&sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);

		memset(sendChat->client, 0, SIZE_OPTION);
		strcpy(sendChat->client, input[2]);
	}
	else if (strcmp(input[0], "logout") == 0)
	{
		fprintf(stdout, "\nGood Bye!\n");
		close(tcpSock);
		exit(1);
	}
	else
		executeHelpCenter(0);

	for (int i = 0; i < 5; i++)
		free(input[i]);
	free(input);
}

int main(int argc, char *argv[])
{
	int multiSock, tcpSock;
	struct sockaddr_in from_addr;

	connectObject tcpInfo;
	chatObject sendChat, recvChat;

	if (argc != 3)
	{
		fprintf(stdout, "Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}

	//# ready for receive connect_info of iomux_multi_plexing_server
	initializeClientMultiSock(&multiSock, argv);

	//# receive iomux connectInfo from multicast server
	char sendMsg[MAX_BUF], recvMsg[MAX_BUF];
	while (true)
	{
		fprintf(stdout, "[SYSTEM] waiting connectInfo from multiCast Server...\n");

		memset(recvMsg, 0, MAX_BUF);
		int add_sz = sizeof(from_addr), str_len;
		if (str_len = recvfrom(multiSock, recvMsg, MAX_BUF, 0, (struct sockaddr *)&from_addr, &add_sz) < 0)
			break;

		convertDataObjectStringToDataObject(recvMsg, &recvData);
		convertConnectStringToConnectObject(recvData.body, &tcpInfo);

		if (tcpInfo.port != -1)
		{
			fprintf(stdout, "[SYSTEM] connectInfo received! %s : %d\n", tcpInfo.ip, tcpInfo.port);
			break;
		}
	}
	close(multiSock);

	//# connect to iomux_multiplexing_server using tcp
	struct sockaddr_in serv_addr;
	tcpSock = socket(PF_INET, SOCK_STREAM, 0);
	if (tcpSock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(tcpInfo.ip);
	serv_addr.sin_port = htons(tcpInfo.port);

	if (connect(tcpSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	//# execute login process using tcp connection
	while (true)
	{
		optionObject login;
		memset(sendMsg, 0, MAX_BUF);
		memset(recvMsg, 0, MAX_BUF);

		fputs("\nPlease input your nickname. If you want to quit, type '/exit' : ", stdout);
		memset(login.argument, 0, SIZE_OPTION);
		fgets(login.argument, SIZE_OPTION, stdin);
		removeEndKeyFromString(login.argument);

		if (!strcmp(login.argument, "/exit") || !strcmp(login.argument, "/exit"))
			exit(1);

		convertOptionObjectToDataObject(COMMAND_LOGIN, &login, &sendData);
		convertDataObjectToDataObjectString(&sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);

		int str_len = read(tcpSock, recvMsg, MAX_BUF);
		if (str_len == -1)
			error_handling("read() error!");

		resultObject result;
		convertDataObjectStringToDataObject(recvMsg, &recvData);
		convertResultStringToResultObject(recvData.body, &result);

		//# print login result
		fprintf(stdout, "==> [Server] : %s\n", result.message);
		if (result.status == RESPONSE_LOGIN_SUCCESS)
			break;
	}

	//# attach iomux function to client tcp socket
	fd_set reads;
	int fd_max, fd_num, str_len, STDIN = fileno(stdin);
	startClientIomuxServer(&reads, &fd_num, &fd_max);

	//# set timeout for select()
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	while (true)
	{
		FD_SET(tcpSock, &reads);
		FD_SET(STDIN, &reads);
		if ((fd_num = select(fd_max + 1, &reads, 0, 0, &timeout)) == -1)
			break;

		for (int sock = 0; sock < fd_max + 1; sock++)
			if (FD_ISSET(sock, &reads))
			{
				if (sock == STDIN)
				{
					memset(recvMsg, 0, MAX_BUF);
					if ((str_len = read(sock, recvMsg, MAX_BUF)) <= 0) 
						exit(1);

					removeEndKeyFromString(recvMsg);
					clientCommandCenter(sendMsg, recvMsg, tcpSock, &sendChat);
					continue;
				}

				memset(recvMsg, 0, MAX_BUF);
				int str_len = read(tcpSock, recvMsg, MAX_BUF);
				if (str_len <= 0)
					error_handling("\n\n[ERROR] Server Connection Lost!");

				convertDataObjectStringToDataObject(recvMsg, &recvData);
				resultObject result;
				switch (recvData.cmdCode)
				{
				case COMMAND_RESULT:
					convertDataObjectStringToDataObject(recvMsg, &recvData);
					convertResultStringToResultObject(recvData.body, &result);

					if (result.status == RESPONSE_CHECK_ONLINE || result.status == RESPONSE_CHECK_OFFLINE ||
						result.status == RESPONSE_GROUP_JOINED || result.status == RESPONSE_GROUP_NOT_JOINED)
					{
						if (result.status == RESPONSE_CHECK_ONLINE)
						{
							fprintf(stdout, "[System] chatRoom with '%s' opened. If you want to quit, type '/exit'\n", sendChat.client);
							chatStatus = CLIENT_STATUS_CHAT;
						}
						else if (result.status == RESPONSE_GROUP_JOINED)
						{
							fprintf(stdout, "[System] chatRoom with group '%s' opened. If you want to quit, type '/exit'\n", sendChat.client);
							chatStatus = CLIENT_STATUS_GROUP;
						}
						else
							fprintf(stdout, "==> [CHECK] : %s\n", result.message);
					}
					else if (result.status == RESPONSE_INFORMATION)
						fprintf(stdout, "==> [Server] : %s\n", result.message);
					else if (result.status == RESPONSE_GROUP_MESSAGE)
						fprintf(stdout, "%s\n", result.message);
					else
						fprintf(stdout, "\n[LINE 316 : RESULT RECV DEFAULT HANDLE] %d / %s\n\n", result.status, result.message);
					break;
				case COMMAND_CHAT:
					convertChatStringToChatObject(recvData.body, &recvChat);
					fprintf(stdout, "> [Message from '%s'] : %s\n", recvChat.client, recvChat.message);
					break;
				default:
					fprintf(stdout, "\n[LINE 323 : SELECT RECV DEFAULT HANDLE] %s\n\n", recvData.body);
					break;
				}
			}
	}

	close(tcpSock);
	return 0;
}
