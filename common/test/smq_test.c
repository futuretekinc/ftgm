#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftm_shared_memory.h"

int	nKey = 1235;
int main(int argc, char *argv[])
{
	FTM_RET	xRet;
	FTM_SMQ_PTR	pSMQ;

	FTM_MEM_init();

	xRet = FTM_SMQ_create(nKey, 1024, 10, &pSMQ);
	if (xRet != FTM_RET_OK)
	{
		printf("SMQ creation failed.\n");
		return	0;	
	}

	FTM_SMQ_print(pSMQ);

	if (argc == 2)
	{
		if (strcasecmp(argv[1], "s") == 0)
		{
			MESSAGE("Server Start!\n");	
			while(1)
			{
				FTM_CHAR	pBuff[1024];
				FTM_ULONG	ulLen = 0;
	
				xRet = FTM_SMQ_pop(pSMQ, pBuff, sizeof(pBuff), &ulLen, 100000);
				if (xRet == FTM_RET_OK)
				{
					printf("Packet received!\n");	
				}
				else
				{
					printf("Queue is empry!\n");	
					sleep(1);
				}
			}
		}
		else if (strcasecmp(argv[1], "c") == 0)
		{
			MESSAGE("Client Start!\n");	
			while(1)
			{
				FTM_CHAR	pBuff[1024];
				FTM_ULONG	ulLen = 0;
				FTM_INT		i;
	
				ulLen = rand() % 1024;
				memset(pBuff, 0, sizeof(pBuff));
				for(i = 0 ; i < ulLen; i++)
				{
					pBuff[i] = '0' + i % 10;
				}
	
	FTM_SMQ_print(pSMQ);
				xRet = FTM_SMQ_push(pSMQ, pBuff, ulLen, 100000);
	FTM_SMQ_print(pSMQ);
				if (xRet == FTM_RET_OK)
				{
					printf("Packet send !\n");	
				}
				else
				{
					printf("Packet send error!\n");	
				}
				sleep(1);
			}
		}
	}	

	xRet = FTM_SMQ_destroy(&pSMQ);

	FTM_MEM_final();

	return	0;
}
