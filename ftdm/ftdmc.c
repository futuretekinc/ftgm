#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftgm_type.h"
#include "ftdm_client.h"
#include "debug.h"


FTGM_STRING	_strPrompt = "FTDMC> ";

int main(int argc , char *argv[])
{
	FTDM_CLIENT	xClient;
	FTGM_BYTE	pSendMessage[1000], pRecvBuff[2000];

	
	if (FTDMC_connect("127.0.0.1", 8888, &xClient) != FTGM_RET_OK)
	{
		perror("connect failed. Error");
		return	0;	
	}

	printf("Connected\n");

	while(1)
	{
		FTGM_INT	nBuffLen = 0;
		printf("%s", _strPrompt);
		fgets((char *)pSendMessage, sizeof(pSendMessage), stdin);

		if (pSendMessage[0] == 'q')
		{
			break;	
		}

		nBuffLen = sizeof(pRecvBuff);
		FTDMC_request(&xClient, pSendMessage, strlen((char*)pSendMessage), pRecvBuff, &nBuffLen, 5000);
	}

	close(xClient.hSock);

	return 0;
}

