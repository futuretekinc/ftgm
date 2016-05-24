#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_node_snmpc.h"
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

	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_final
(
	FTOM_NODE_FINSC_PTR pNode
)
{
	ASSERT(pNode != NULL);

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
	FTM_INT	nRet;
	FTM_RET	xRet;
	struct sockaddr_in ws_addr, cv_addr;
	FTM_CHAR	pFINSReq[2048];
	FTM_CHAR	pFINSResp[2048];
	socklen_t	nAddrLen;
	FTM_INT		nRecvLen;

	FTM_LOCK_set(&pNode->xLock);
	memset(&ws_addr, 0, sizeof(ws_addr));
	ws_addr.sin_family = AF_INET;
	ws_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ws_addr.sin_port  = htons(9600);

	nRet = bind(pNode->xSockFD, (struct sockaddr *)&ws_addr, sizeof(ws_addr));
	if (nRet < 0)
	{
		ERROR("Can't bind local address!\n");	
		xRet = FTM_RET_ERROR;
		goto finish;
	}

	pFINSReq[0] = 0x80;
	pFINSReq[1] = 0x00;
	pFINSReq[2] = 0x02;
	pFINSReq[3] = 0x00;
	pFINSReq[4] = 0x00;
	pFINSReq[5] = 0x00;
	pFINSReq[6] = 0x00;
	pFINSReq[7] = 0x02;
	pFINSReq[8] = 0x00;
	pFINSReq[9] = 0x0e;
	pFINSReq[10] = 0x01;
	pFINSReq[11] = 0x01;
	pFINSReq[12] = 0x82;
	pFINSReq[13] = 0x21;
	pFINSReq[14] = 0x34;
	pFINSReq[15] = 0x00;
	pFINSReq[16] = 0x00;
	pFINSReq[17] = 0x03;

	memset(&cv_addr, 0, sizeof(cv_addr));
	cv_addr.sin_family = AF_INET;
	cv_addr.sin_addr.s_addr = inet_addr("191.30.5.178");
	cv_addr.sin_port = htons(9600);

	if (sendto(pNode->xSockFD, pFINSReq, 18, 0, (const struct sockaddr *)&cv_addr, sizeof(cv_addr)) == 18)
	{
		TRACE("Send success!\n");		
	}

	nRecvLen = recvfrom(pNode->xSockFD, pFINSResp, sizeof(pFINSResp), 0, (struct sockaddr *)&cv_addr, &nAddrLen);
	if (nRecvLen > 0)
	{
		FTM_INT	i;

		TRACE("Recv success : %d !\n", nRecvLen);		
		for(i = 0 ; i < nRecvLen ; i++)
		{
			MESSAGE("%02x ", pFINSResp[i]);	
		}
		MESSAGE("\n");
	
	}

	close(pNode->xSockFD);

finish:
	FTM_LOCK_reset(&pNode->xLock);

	return	FTM_RET_OK;
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

