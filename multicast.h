//import define.h for shared-environment
#include "define.h"

//# start multiCast server and setup connect info
void initializeServerMultiSock(int *multi_sock, struct sockaddr_in *multiaddr, connectObject *connectInfo, char **argv)
{
	struct ifreq ifr;

	memset(connectInfo->ip, 0, SIZE_IP);
	int ip_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

	if (ioctl(ip_sock, SIOCGIFADDR, &ifr) < 0)
		perror("Error ");
	else
		inet_ntop(AF_INET, ifr.ifr_addr.sa_data + 2, connectInfo->ip, SIZE_IP);

	srand(time(NULL));
	connectInfo->port = rand() % 1000 + 5000;

	*multi_sock = socket(PF_INET, SOCK_DGRAM, 0); // Create UDP Socket
	memset(multiaddr, 0, sizeof(multiaddr));
	multiaddr->sin_family = AF_INET;
	multiaddr->sin_addr.s_addr = inet_addr(argv[1]); //Multicast IP
	multiaddr->sin_port = htons(atoi(argv[2]));		 //Multicast Port
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
