#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

static 
FTM_RET	FTOM_NODE_FINSC_HHTW_get
(
	FTOM_NODE_FINSC_PTR	pFINSC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static 
FTM_RET	FTOM_NODE_FINSC_HHTW_set
(
	FTOM_NODE_FINSC_PTR	pFINSC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static 
FTM_RET	FTOM_NODE_FINSC_getEPData
(
	FTOM_NODE_FINSC_PTR	pFINSC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static 
FTM_RET	FTOM_NODE_FINSC_setEPData
(
	FTOM_NODE_FINSC_PTR	pFINSC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static
FTOM_NODE_FINSC_DESCRIPT	pClient[] =
{
	{
		.pModel	= "hhtw comp",
		.fGet	= FTOM_NODE_FINSC_HHTW_get,
		.fSet	= FTOM_NODE_FINSC_HHTW_set
	}
};

FTM_RET	FTOM_NODE_FINSC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTOM_NODE_FINSC_PTR	pNode;
	

	pNode = (FTOM_NODE_FINSC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_FINSC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.xDescript.xType		= FTOM_NODE_TYPE_FINSC;
	pNode->xCommon.xDescript.fInit		= (FTOM_NODE_INIT)FTOM_NODE_FINSC_init;
	pNode->xCommon.xDescript.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_FINSC_final;
	pNode->xCommon.xDescript.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_FINSC_getEPData;
	pNode->xCommon.xDescript.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_FINSC_setEPData;
	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_destroy
(
	FTOM_NODE_FINSC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_init
(
	FTOM_NODE_FINSC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_init(&pNode->xLock);
	FTM_LIST_init(&pNode->xCommon.xEPList);

	pNode->xSockFD = socket(AF_INET, SOCK_DGRAM, 0);
	if (pNode->xSockFD < 0)
	{
		ERROR("Can't open datagram socket!\n");
		return	FTM_RET_ERROR;
	}

	memset(&pNode->xLocal, 0, sizeof(pNode->xLocal));
	pNode->xLocal.sin_family 		= AF_INET;
	pNode->xLocal.sin_addr.s_addr 	= htonl(INADDR_ANY);
	pNode->xLocal.sin_port  		= htons(pNode->xCommon.xInfo.xOption.xFINS.ulSP);

	memset(&pNode->xRemote, 0, sizeof(pNode->xRemote));
	pNode->xRemote.sin_family 		= AF_INET;
	pNode->xRemote.sin_addr.s_addr 	= inet_addr(pNode->xCommon.xInfo.xOption.xFINS.pDIP);
	pNode->xRemote.sin_port 		= htons(pNode->xCommon.xInfo.xOption.xFINS.ulDP);

	nRet = bind(pNode->xSockFD, (struct sockaddr *)&pNode->xLocal, sizeof(pNode->xLocal));
	if (nRet < 0)
	{
		ERROR("Can't bind local address!\n");	
		close(pNode->xSockFD);
		pNode->xSockFD = -1;

		return	FTM_RET_ERROR;
	}

	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_final
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

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_getEPData
(
	FTOM_NODE_FINSC_PTR 	pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_INT		i;

	for(i = 0 ; i < sizeof(pClient) / sizeof(FTOM_NODE_FINSC_DESCRIPT) ; i++)
	{
		if (strcasecmp(pNode->xCommon.xInfo.xOption.xMB.pModel, pClient[i].pModel) == 0)
		{
			return	pClient[i].fGet(pNode, pEP, pData);
		
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTOM_NODE_FINSC_setEPData
(
	FTOM_NODE_FINSC_PTR 	pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_INT		i;

	for(i = 0 ; i < sizeof(pClient) / sizeof(FTOM_NODE_FINSC_DESCRIPT) ; i++)
	{
		if (strcasecmp(pNode->xCommon.xInfo.xOption.xMB.pModel, pClient[i].pModel) == 0)
		{
			return	pClient[i].fSet(pNode, pEP, pData);
		
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

static 
FTM_RET	FTOM_NODE_FINSC_HHTW_get
(
	FTOM_NODE_FINSC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	FTM_RET	xRet;

	FTM_CHAR	pFINSReq[128];
	FTM_CHAR	pFINSResp[128];
	FTM_INT		nReqLen = 0;
	FTM_INT		nRecvLen = 0;
	FTM_BOOL	bDone = FTM_FALSE;
	FTM_ULONG	usValue = 0;
	FTM_INT		nIndex = 0;

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
	pFINSReq[nReqLen++] = 0x0D;

	FTM_LOCK_set(&pNode->xLock);

	FTM_INT	nRetry = pNode->xCommon.xInfo.xOption.xFINS.ulRetryCount;

	while(!bDone && (nRetry > 0))
	{
		if (sendto(pNode->xSockFD, pFINSReq, nReqLen, 0, (const struct sockaddr *)&pNode->xRemote, sizeof(pNode->xRemote)) != nReqLen)
		{
			--nRetry;
			continue;
		}

		FTM_INT	nTimeout = pEP->xInfo.ulTimeout;
		while(--nTimeout > 0)
		{
			nRecvLen = recv(pNode->xSockFD, pFINSResp, sizeof(pFINSResp), MSG_DONTWAIT);
			if (nRecvLen > 0)
			{
				break;
			}
			usleep(1000);
		}

		if (nRecvLen == 14 + pFINSReq[nReqLen-1]*2)
		{
			if ((pFINSReq[3] == pFINSResp[6]) 
				&& (pFINSReq[4] ==  pFINSResp[7]) 
				&& (pFINSReq[5] ==  pFINSResp[8]) 
				&& (pFINSReq[9] == pNode->xCommon.xInfo.xOption.xFINS.ulServerID))
			{
				bDone = FTM_TRUE;	
				break;
			}
		}
		
		--nRetry;
	}

	if (!bDone)
	{
		xRet = FTM_RET_COMM_TIMEOUT;
		goto finish;	
	}

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
	usValue = (((FTM_USHORT)(FTM_UINT8)pFINSResp[14 + nIndex*2]) << 8) | (FTM_USHORT)(FTM_UINT8)pFINSResp[14+nIndex*2+1];

	pData->ulTime = time(NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->xType  = FTM_VALUE_TYPE_FLOAT;
	xRet = FTM_VALUE_initFLOAT(&pData->xValue, (FTM_FLOAT)usValue / 100.0);

finish:
	FTM_LOCK_reset(&pNode->xLock);

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

