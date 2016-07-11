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
	FTM_SMP_PTR	pSMP;

	FTM_MEM_init();

	if (argc == 2)
	{
		if (strcasecmp(argv[1], "s") == 0)
		{
			MESSAGE("Server Start!\n");	
			xRet = FTM_SMP_createServer(nKey, &pSMP);
			if (xRet != FTM_RET_OK)
			{
				printf("SMP creation failed.\n");
				return	0;	
			}

			FTM_SMP_print(pSMP);

			while(1)
			{
				FTM_CHAR	pBuff[1024];
				FTM_ULONG	ulLen = 0;
	
				xRet = FTM_SMP_receiveReq(pSMP, pBuff, sizeof(pBuff), &ulLen, 100000);
				if (xRet == FTM_RET_OK)
				{
					printf("Packet received!\n");	
					FTM_SMP_sendResp(pSMP, pBuff, ulLen, 100000);
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
			xRet = FTM_SMP_createClient(nKey, &pSMP);
			if (xRet != FTM_RET_OK)
			{
				printf("SMP creation failed.\n");
				return	0;	
			}

			FTM_SMP_print(pSMP);

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
	
	FTM_SMP_print(pSMP);
				xRet = FTM_SMP_call(pSMP, pBuff, ulLen, pBuff, sizeof(pBuff), &ulLen, 100000);
	FTM_SMP_print(pSMP);
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

	xRet = FTM_SMP_destroy(&pSMP);

	FTM_MEM_final();

	return	0;
}
