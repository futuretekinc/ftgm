#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_client.h"
#include "debug.h"


FTDM_CHAR_PTR _strPrompt = "FTDMC> ";

int main(int argc , char *argv[])
{
	FTDM_CLIENT	xClient;
	FTDM_BYTE	pSendMessage[1000], pRecvBuff[2000];

	
	if (FTDMC_connect(inet_addr("127.0.0.1"), 8888, &xClient) != FTDM_RET_OK)
	{
		perror("connect failed. Error");
		return	0;	
	}

	printf("Connected\n");

	while(1)
	{
		FTDM_INT	nBuffLen = 0;
		printf("%s", _strPrompt);
		fgets((char *)pSendMessage, sizeof(pSendMessage), stdin);

		if (pSendMessage[0] == 'q')
		{
			break;	
		}

		nBuffLen = sizeof(pRecvBuff);
	}

	close(xClient.hSock);

	return 0;
}

