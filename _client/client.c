#include <stdio.h>
#include <stdlib.h>

//# share profile with server
#include "../define.h"

int split(char *str, char c, char ***arr)
{
	int count = 1;
	int token_len = 1;
	int i = 0;
	char *p;
	char *t;

	p = str;
	// strcpy(p, str);

	while (*p != '\0')
	{
		if (*p == c)
			count++;
		p++;
	}

	*arr = (char **)malloc(sizeof(char *) * count);
	if (*arr == NULL)
		exit(1);

	p = str;
	while (*p != '\0')
	{
		if (*p == c)
		{
			(*arr)[i] = (char *)malloc(sizeof(char) * token_len);
			if ((*arr)[i] == NULL)
				exit(1);

			token_len = 0;
			i++;
		}
		p++;
		token_len++;
	}
	(*arr)[i] = (char *)malloc(sizeof(char) * token_len);
	if ((*arr)[i] == NULL)
		exit(1);

	i = 0;
	p = str;
	t = ((*arr)[i]);
	while (*p != '\0')
	{
		if (*p != c && *p != '\0')
		{
			*t = *p;
			t++;
		}
		else
		{
			*t = '\0';
			i++;
			t = ((*arr)[i]);
		}
		p++;
	}

	return count;
}

void removeEndKeyFromString(char *recvMsg)
{
	int i = 0;
	while (true)
	{
		if (recvMsg[i])
			i++;
		else
			break;
	}
	recvMsg[i - 1] = '\0';
}

void initializeClientMultiSock(int *multiSock, char **argv)
{
	struct sockaddr_in addr;
	struct ip_mreq join_addr;

	// Create UDP Socket
	*multiSock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[2]));

	int on = 1;
	setsockopt(*multiSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	// Bind socket
	if (bind(*multiSock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		error_handling("bind() error");
		close(*multiSock);
		exit(1);
	}

	// Specify the multicast Group
	join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	// Accept multicast from any interface
	join_addr.imr_interface.s_addr = htonl(INADDR_ANY);

	// Join Multicast Group
	if ((setsockopt(*multiSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&join_addr, sizeof(join_addr))) < 0)
	{
		error_handling("SetsockOpt Join Error \n");
		close(*multiSock);
		exit(1);
	}
}

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
		printf("bind() error");
		exit(0);
	}
	if (listen(clientIomuxSock, 5) == -1)
	{
		printf("listen() error");
		exit(0);
	}

	FD_ZERO(reads);
	FD_SET(clientIomuxSock, reads);
	*fd_max = clientIomuxSock;

	fprintf(stdout, "\n[SYSTEM] client TCP one-way multiPlexing Server Starting ... on %d\n", clientPort);
}

void executeHelpCenter(int helpCode) {
	/*
		all : 0
		chat : 1
		check : 2
	*/
	switch(helpCode) {

	}
}

void clientCommandCenter(dataObject *sendData, dataObject *recvData, char *sendMsg, char *recvMsg, int tcpSock, bool *chatStatus, chatObject *sendChat)
{
	char **input = NULL;
	int columnCount = split(recvMsg, ' ', &input);

	// select() in chat status
	if (*chatStatus) {
		if (strcmp(recvMsg, "/exit") == 0) {
			fprintf(stdout, "[System] chatRoom with '%s' closed.\n", sendChat->client);
			*chatStatus = false;
			return;
		}
		
		memset(sendChat->message, 0, SIZE_MESSAGE);
		memcpy(sendChat->message, recvMsg, SIZE_MESSAGE);
			
		memset(sendMsg, 0, MAX_BUF);
		convertChatObjectToDataObject(sendChat, sendData);
		convertDataObjectToDataObjectString(sendData, sendMsg);
		write(tcpSock, sendMsg, MAX_BUF);
	}
	else if (strcmp(input[0], "chat") == 0)
	{
		if (columnCount != 2) {
			executeHelpCenter(1);
			return;
		}
		
		memset(sendChat->client, 0, SIZE_OPTION);
		strcpy(sendChat->client, input[1]);

		//# open chat room to target
		fprintf(stdout, "[System] chatRoom with '%s' opened. If you want to quit, type '/exit'\n", sendChat->client);
		*chatStatus = true;
	}
	else if(false) {

	}
	else if(false) {

	}
	else if(strcmp(input[0], "logout") == 0) {
		fprintf(stdout, "\nGood Bye!\n");
		close(tcpSock);
		exit(1);
	}
	else executeHelpCenter(0);
}

int main(int argc, char *argv[])
{
	int multiSock, tcpSock;
	struct sockaddr_in from_addr;
	
	bool chatStatus = false;
	connectObject tcpInfo;
	dataObject sendData, recvData;
	chatObject sendChat, recvChat;

	if (argc != 3)
	{
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
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
			printf("[SYSTEM] connectInfo received! %s : %d\n", tcpInfo.ip, tcpInfo.port);
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

		//print login result
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
					if ((str_len = read(sock, recvMsg, MAX_BUF)) <= 0) // close request!
						exit(1);

					removeEndKeyFromString(recvMsg);
					clientCommandCenter(&sendData, &recvData, sendMsg, recvMsg, tcpSock, &chatStatus, &sendChat);
					continue;
				}

				int str_len = read(tcpSock, recvMsg, MAX_BUF);
				if (str_len < 0)
					error_handling("read() error!");

				convertDataObjectStringToDataObject(recvMsg, &recvData);
				switch(recvData.cmdCode) {
					case COMMAND_CHAT:
						convertChatStringToChatObject(recvData.body, &recvChat);
						printf("> [Message from '%s'] : %s\n", recvChat.client, recvChat.message);
						break;
					default:
						fprintf(stdout, "\n\n[LINE 343 : SELECT RECV DEFAULT HANDLE] %s\n\n", recvData.body);
						break;
				}
			}
	}

	close(tcpSock);
	return 0;
}
