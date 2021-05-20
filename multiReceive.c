#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "define.h"

int main(int argc, char *argv[])
{
	int recv_sock;
	int str_len;
	char buf[BUF_SIZE+1];
	struct sockaddr_in addr;
	struct sockaddr_in from_addr;
	struct ip_mreq join_addr;
	
	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	 }
 
       // Create UDP Socket	
	recv_sock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);	
	addr.sin_port=htons(atoi(argv[2]));

	int on=1;
       setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

// Bind socket	
	if(bind(recv_sock, (struct sockaddr*) &addr, sizeof(addr))==-1)
	{
		printf("bind() error");
		close(recv_sock);
		exit(1);	
	}

	// Specify the multicast Group	
	join_addr.imr_multiaddr.s_addr=inet_addr(argv[1]);
	// Accept multicast from any interface
	join_addr.imr_interface.s_addr=htonl(INADDR_ANY);
  
// Join Multicast Group	
	if ( (setsockopt(recv_sock, IPPROTO_IP, 
		IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)))< 0 ) 
	{
		printf(" SetsockOpt Join Error \n");
		close(recv_sock);
		exit(1);
	}
  
	while(1)
	{
		memset(buf, 0, BUF_SIZE);

		int add_sz = sizeof(from_addr);
		str_len=recvfrom(recv_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&from_addr, &add_sz);
		dataObject receive;
        convertDataObjectStringToDataObject(buf, &receive);
        
        connectObject tcpInfo;
        convertConnectStringToConnectObject(receive.body, &tcpInfo);
        

		if(str_len < 0) break;
		
		printf("FROM [%s] : %s", (char*)inet_ntoa((struct in_addr)from_addr.sin_addr), buf);
	}
	close(recv_sock);
	
	return 0;
}
