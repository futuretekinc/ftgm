#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_message_queue.h"
#include "ftom_node_class.h"
#include "ftom_node_fins_client.h"

#define	FTOM_NODE_FINSC_HHTW_REG_COUNT	14

typedef	struct	FTOM_NODE_FINSC_HHTW_DATA_STRUCT
{
	pthread_t	xDataProcess;
	FTM_ULONG	ulTime;
	FTM_BOOL	bValid;
	FTM_USHORT	pusRegs[FTOM_NODE_FINSC_HHTW_REG_COUNT];
}	FTOM_NODE_FINSC_HHTW_DATA, _PTR_ FTOM_NODE_FINSC_HHTW_DATA_PTR;
static
FTM_VOID_PTR	FTOM_NODE_FINSC_HHTW_dataProcess
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTOM_NODE_FINSC_HHTW_init
(
	FTOM_NODE_FINSC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->pData == NULL)
	{
		pNode->pData = (FTOM_NODE_FINSC_HHTW_DATA_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_FINSC_HHTW_DATA));
		if (pNode->pData == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	}

	memset(&pNode->xLocal, 0, sizeof(pNode->xLocal));
	pNode->xLocal.sin_family 		= AF_INET;
	pNode->xLocal.sin_addr.s_addr 	= htonl(INADDR_ANY);
	pNode->xLocal.sin_port  		= htons(pNode->xCommon.xInfo.xOption.xFINS.ulSP);

	memset(&pNode->xRemote, 0, sizeof(pNode->xRemote));
	pNode->xRemote.sin_family 		= AF_INET;
	pNode->xRemote.sin_addr.s_addr 	= inet_addr(pNode->xCommon.xInfo.xOption.xFINS.pDIP);
	pNode->xRemote.sin_port 		= htons(pNode->xCommon.xInfo.xOption.xFINS.ulDP);

	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_HHTW_final
(
	FTOM_NODE_FINSC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->xSockFD > 0)
	{
		close(pNode->xSockFD);
		pNode->xSockFD = -1;	
	}

	if (pNode->pData != NULL)
	{
		FTM_MEM_free(pNode->pData);
		pNode->pData = NULL;
	}

	return	FTM_RET_OK;
}

static
FTM_RET	FTOM_NODE_FINSC_HHTW_preStart
(
	FTOM_NODE_FINSC_PTR	pNode
)
{
	ASSERT(pNode != NULL);
	
	pNode->xSockFD = socket(AF_INET, SOCK_DGRAM, 0);
	if (pNode->xSockFD < 0)
	{
		ERROR2(FTM_RET_ERROR, "Can't open datagram socket!\n");
		return	FTM_RET_ERROR;
	}
	nRet = bind(pNode->xSockFD, (struct sockaddr *)&pNode->xLocal, sizeof(pNode->xLocal));
	if (nRet < 0)
	{
		ERROR2(FTM_RET_ERROR, "Can't bind local address!\n");	
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

static
FTM_RET	FTOM_NODE_FINSC_HHTW_postStop
(
	FTOM_NODE_FINSC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->xSockFD > 0)
	{
		close(pNode->xSockFD);
		pNode->xSockFD = 0;
	}

	return	FTM_RET_OK;
}

static
FTM_VOID_PTR FTOM_NODE_FINSC_HHTW_process
(
	FTOM_NODE_FINSC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTM_INT			nRet;
	FTOM_MSG_PTR	pMsg;
	FTM_TIMER		xReportTimer;
	FTOM_NODE_FINSC_HHTW_DATA_PTR	pData = (FTOM_NODE_FINSC_HHTW_DATA_PTR)pNode->pData;

	pNode->xCommon.xState = FTOM_NODE_STATE_RUN;

	TRACE("Node[%s] start(FINS Client).\n", pNode->xCommon.xInfo.pDID);

	pNode->xCommon.bStop = FTM_FALSE;
	xRet = FTM_TIMER_initS(&xReportTimer, pNode->xCommon.xInfo.ulReportInterval);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Report timer init failed!\n");	
	}

	nRet = pthread_create(&pData->xDataProcess, NULL, FTOM_NODE_FINSC_HHTW_dataProcess, pNode);
	if (nRet < 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_THREAD, "HHTW data process not started!\n");	
	}

	while(!pNode->xCommon.bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
	
		FTM_TIMER_remainMS(&xReportTimer, &ulRemainTime);
		while (!pNode->xCommon.bStop && (FTOM_MSGQ_timedPop(&pNode->xCommon.xMsgQ, ulRemainTime, &pMsg) == FTM_RET_OK))
		{
			TRACE("Message received[%08x]\n", pMsg->xType);
			switch(pMsg->xType)
			{
			case	FTOM_MSG_TYPE_QUIT:
				{	
					pNode->xCommon.bStop = FTM_TRUE;
				}
				break;

			default:
				{
					WARN("Invalid message[%08x]\n", pMsg->xType);	
				}
			}
			FTM_MEM_free(pMsg);

			FTM_TIMER_remainMS(&xReportTimer, &ulRemainTime);
		}
	
		if (!pNode->xCommon.bStop)
		{
			FTM_TIMER_waitForExpired(&xReportTimer);
		}

		xRet = FTM_TIMER_addS(&xReportTimer, pNode->xCommon.xInfo.ulReportInterval);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Report timer update failed!\n");	
		}
	} 

	TRACE("Node[%s] stopped.\n", pNode->xCommon.xInfo.pDID);

	return	FTM_RET_OK;
}

static 
FTM_VOID_PTR	FTOM_NODE_FINSC_HHTW_dataProcess
(
	FTM_VOID_PTR	pData
)
{
	FTM_CHAR	pFINSReq[20];
	FTM_CHAR	pFINSResp[14 + FTOM_NODE_FINSC_HHTW_REG_COUNT*2];
	FTM_INT		nReqLen = 0;
	FTM_INT		nRecvLen= 0;
	FTOM_NODE_FINSC_PTR	pNode = (FTOM_NODE_FINSC_PTR)pData;
	FTOM_NODE_FINSC_HHTW_DATA_PTR pHHTWData = (FTOM_NODE_FINSC_HHTW_DATA_PTR)pNode->pData;

	pFINSReq[nReqLen++] = 0x80;
	pFINSReq[nReqLen++] = 0x00;
	pFINSReq[nReqLen++] = 0x02;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulDA >> 16) & 0xFF;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulDA >>  8) & 0xFF;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulDA >>  0) & 0xFF;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulSA >> 16) & 0xFF;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulSA >>  8) & 0xFF;
	pFINSReq[nReqLen++] = (pNode->xCommon.xInfo.xOption.xFINS.ulSA >>  0) & 0xFF;
	pFINSReq[nReqLen++] = pNode->xCommon.xInfo.xOption.xFINS.ulServerID;
	pFINSReq[nReqLen++] = 0x01;
	pFINSReq[nReqLen++] = 0x01;
	pFINSReq[nReqLen++] = 0x82;
	pFINSReq[nReqLen++] = 0x24;
	pFINSReq[nReqLen++] = 0x54;
	pFINSReq[nReqLen++] = 0x00;
	pFINSReq[nReqLen++] = 0x00;
	pFINSReq[nReqLen++] = FTOM_NODE_FINSC_HHTW_REG_COUNT;

	while(!pNode->xCommon.bStop)
	{
		FTM_LOCK_set(pNode->pLock);

		FTM_BOOL	bDone = FTM_FALSE;
	
		memset(pFINSResp, 0, sizeof(pFINSResp));

		if (sendto(pNode->xSockFD, pFINSReq, nReqLen, 0, (const struct sockaddr *)&pNode->xRemote, sizeof(pNode->xRemote)) == nReqLen)
		{
			pNode->xCommon.xStatistics.ulTxCount++;		

			FTM_INT	nTimeout = 100;
			while(--nTimeout > 0)
			{
				nRecvLen = recv(pNode->xSockFD, pFINSResp, sizeof(pFINSResp), MSG_DONTWAIT);
				if (nRecvLen != 0)
				{
					break;
				}
				usleep(1000);
			}

			if (nRecvLen > 0)
			{
				pNode->xCommon.xStatistics.ulRxCount++;		

				if (nRecvLen == 14 + FTOM_NODE_FINSC_HHTW_REG_COUNT * 2)
				{

					if ((pFINSReq[3] == pFINSResp[6]) 
						&& (pFINSReq[4] ==  pFINSResp[7]) 
						&& (pFINSReq[5] ==  pFINSResp[8]) 
						&& (pFINSReq[9] == pNode->xCommon.xInfo.xOption.xFINS.ulServerID))
					{
						bDone = FTM_TRUE;	
					}
					else
					{
						pNode->xCommon.xStatistics.ulInvalidFrame++;		
					}
				}
			}
			else if (nRecvLen < 0)
			{
				pNode->xCommon.xStatistics.ulRxError++;		
			}
		}
		else
		{
			pNode->xCommon.xStatistics.ulTxError++;		
		}
	
		if (bDone)
		{
			FTM_INT		nIndex 	= 0;

			pHHTWData->ulTime = time(NULL);
			pHHTWData->bValid = FTM_TRUE;
			for(nIndex = 0 ; nIndex < FTOM_NODE_FINSC_HHTW_REG_COUNT; nIndex++)
			{
				pHHTWData->pusRegs[nIndex] = (((FTM_USHORT)(FTM_UINT8)pFINSResp[14 + nIndex*2]) << 8) | (FTM_USHORT)(FTM_UINT8)pFINSResp[14+nIndex*2+1];
			}
		}
		else
		{
			if (pHHTWData->ulTime + pNode->xCommon.xInfo.ulTimeout > time(NULL))
			{
				pHHTWData->ulTime = time(NULL);
				pHHTWData->bValid = FTM_FALSE;
				WARN("HHTW request timeout!\n");
			}
		}

		FTM_LOCK_reset(pNode->pLock);
		usleep(100000);
	}

	return	FTM_RET_OK;
}

static 
FTM_RET	FTOM_NODE_FINSC_HHTW_get
(
	FTOM_NODE_FINSC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	FTM_RET		xRet;
	FTM_INT		nIndex;
	FTOM_NODE_FINSC_HHTW_DATA_PTR pHHTWData = (FTOM_NODE_FINSC_HHTW_DATA_PTR)pNode->pData;

	switch(pEP->xInfo.xDEPID & 0x7F000000)
	{
	case	FTM_EP_TYPE_PRESSURE:
		{
			switch(pEP->xInfo.xDEPID & 0xFF)
			{
			case	1: nIndex = 0; break;
			case	2: nIndex = 2; break;
			case	3: nIndex = 4; break;
			case	4: nIndex = 10; break;
			case	5: nIndex = 12; break;
			default:
				xRet = FTM_RET_OBJECT_NOT_FOUND;	
				goto finish;
			}
		}
		break;

	case	FTM_EP_TYPE_AI:
		{
			switch(pEP->xInfo.xDEPID & 0xFF)
			{
			case	1: nIndex = 6; break;
			case	2: nIndex = 8; break;
			default:
				xRet = FTM_RET_OBJECT_NOT_FOUND;	
				goto finish;
			}
		}
		break;

	default:
		{
			xRet = FTM_RET_OBJECT_NOT_FOUND;	
			goto finish;
		}

	}


	if (pHHTWData->bValid)
	{
		pData->ulTime = pHHTWData->ulTime;
		pData->xState = FTM_EP_DATA_STATE_VALID;
		xRet = FTM_VALUE_initFLOAT(&pData->xValue, (FTM_FLOAT)pHHTWData->pusRegs[nIndex] / 100.0);
	}
	else
	{
		pData->xState = FTM_EP_DATA_STATE_INVALID;
	}

finish:
	FTM_LOCK_reset(pNode->pLock);

	return	xRet;
}


FTM_RET	FTOM_NODE_FINSC_HHTW_set
(
	FTOM_NODE_FINSC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	return	FTM_RET_OK;
}


FTOM_NODE_CLASS	xHHTWNodeClass = 
{
	.pModel		= "hhtw comp",
	.xType		= FTOM_NODE_TYPE_FINSC,
	.fCreate	= (FTOM_NODE_CREATE)FTOM_NODE_FINSC_create,
	.fDestroy	= (FTOM_NODE_DESTROY)FTOM_NODE_FINSC_destroy,
	.fInit		= (FTOM_NODE_INIT)FTOM_NODE_FINSC_HHTW_init,
	.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_FINSC_HHTW_final,
	.fPrestart	= (FTOM_NODE_PRESTART)FTOM_NODE_FINSC_HHTW_preStart,
	.fProcess	= (FTOM_NODE_PROCESS)FTOM_NODE_FINSC_HHTW_process,
	.fPoststop	= (FTOM_NODE_POSTSTOP)FTOM_NODE_FINSC_HHTW_postStop,
	.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_FINSC_HHTW_get,
	.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_FINSC_HHTW_set
};

