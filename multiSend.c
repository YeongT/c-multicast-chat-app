#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "define.h"

int main(int argc, char *argv[])
{
  struct ifreq ifr;
  int ip_sock;
	int serv_sock;
	char message[MAX_BUF]={0, };
	struct sockaddr_in mul_addr;
	struct ip_mreq join_addr;
	socklen_t  mul_addr_sz;
	connectObject connectInfo;

  memset(connectInfo.ip, 0, 15);

	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}

  ip_sock = socket(AF_INET, SOCK_DGRAM, 0);
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

  if (ioctl(ip_sock, SIOCGIFADDR, &ifr) < 0)
    printf("Error");
  else
    inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, connectInfo.ip, 15);

  	connectInfo.port = atoi(argv[2]);
  	printf("%s:%d\n", connectInfo.ip, connectInfo.port);

	serv_sock=socket(PF_INET, SOCK_DGRAM, 0); // Create UDP Socket
 	memset(&mul_addr, 0, sizeof(mul_addr));
	mul_addr.sin_family=AF_INET;
	mul_addr.sin_addr.s_addr=inet_addr(argv[1]); //Multicast IP
	mul_addr.sin_port=htons(atoi(argv[2])); //Multicast Port
	
	int time_live=2;
	setsockopt(serv_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));


	// Specify the multicast Group	
	join_addr.imr_multiaddr.s_addr=inet_addr(argv[1]);
	// Accept multicast from any interface
	join_addr.imr_interface.s_addr=htonl(INADDR_ANY);
  
	
	dataObject broadCast;
	convertConnectObjectToDataObject(&connectInfo, &broadCast);
	convertDataObjectToDataObjectString(&broadCast, message);

	while(1)
	{
		sendto(serv_sock, message, MAX_BUF, 0, (struct sockaddr *)&mul_addr, sizeof(mul_addr));
		fprintf(stdout, "Message Sending...\n");
	}
	close(serv_sock);

	return 0;
}

