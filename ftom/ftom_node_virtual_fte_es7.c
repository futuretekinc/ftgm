#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_virtual.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

typedef	struct
{
	pthread_t	xThread;
	FTM_BOOL	bStop;
	FTM_BOOL	bDI;
	FTM_BOOL	bDO;
	FTM_ULONG	ulCO2;
	FTM_FLOAT	fHumidity;
	FTM_FLOAT	pTemperatures[11];
}	FTOM_NODE_VIRTUAL_FTE_ES7_DATA, _PTR_ FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR;


static
FTM_VOID_PTR	FTOM_NODE_VIRTUAL_FTE_ES7_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR pES7 = (FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR)pData;
	FTM_TIMER	xUpdateTimer;
	FTM_ULONG	ulLoop = 0;
	FTM_ULONG	ulRemainTime = 0;
	FTM_INT		i;

	pES7->bStop = FTM_FALSE;
	pES7->bDI = FTM_FALSE;
	pES7->bDO = FTM_FALSE;
	pES7->ulCO2 = 400;
	pES7->fHumidity = 60.0;
	for(i = 0  ; i < 11 ; i++)
	{
		pES7->pTemperatures[i] = 20.0;	
	}

	FTM_TIMER_initS(&xUpdateTimer, 100);
	while(!pES7->bStop)
	{
		if (++ulLoop >= 10)
		{
			pES7->bDI = (rand() % 100 >= 80);
			ulLoop  = 0;	
		}
		
		pES7->ulCO2 += (rand() % 21) - 10;
		if (pES7->ulCO2 > 100000)
		{
			pES7->ulCO2 = 0;
		}
		else if (pES7->ulCO2 > 10000)
		{
			pES7->ulCO2 = 10000;
		}

		pES7->fHumidity += ((rand() % 21) - 10) / 10.0;
		if (pES7->fHumidity < 0)
		{
			pES7->fHumidity = 0;
		}
		else if (pES7->fHumidity > 100)
		{
			pES7->fHumidity = 100;
		}

		for(i = 0 ; i < 11 ; i++)
		{
			pES7->pTemperatures[i] += ((rand() % 21) - 10) / 10.0;
			if (pES7->pTemperatures[i] < -20)
			{
					pES7->pTemperatures[i]= -20;
			}
			else if (pES7->pTemperatures[i] > 40)
			{
					pES7->pTemperatures[i] = 40;
			}
		}

		FTM_TIMER_remainMS(&xUpdateTimer, &ulRemainTime);
		usleep(ulRemainTime * 1000);

		FTM_TIMER_addMS(&xUpdateTimer, 100);
	}

	return	0;
}

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_init
(
	FTOM_NODE_VIRTUAL_PTR pNode
)
{
	FTM_RET				xRet;
	FTM_ULONG			ulEPCount;
	FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR	pES7;

	ASSERT(pNode != NULL);

	xRet = FTM_LOCK_create(&pNode->pLock);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Lock init failed!\n");
		return	xRet;	
	}

	xRet = FTOM_NODE_getEPCount((FTOM_NODE_PTR)pNode, &ulEPCount);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Node[%s] get EP count failed.!\n", pNode->xCommon.xInfo.pDID);
		return	xRet;	
	}

	TRACE("NODE(%08x)[%s] has %d EPs\n", pNode, pNode->xCommon.xInfo.pDID, ulEPCount);
	if (ulEPCount != 0)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTOM_EP_PTR				pEP;
			FTOM_EP_CLASS_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTOM_NODE_getEPAt((FTOM_NODE_PTR)pNode, i, (FTOM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTOM_EP_CLASS_get((pEP->xInfo.xType & FTM_EP_TYPE_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%s] information not found\n", pEP->xInfo.pEPID);
				continue;
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", 
				pNode->xCommon.xInfo.xOption.xSNMP.pMIB, 
				pEPClassInfo->xInfo.pValue);
			pEP->xOption.xSNMP.xOID.nLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->xOption.xSNMP.xOID.pIDs, &pEP->xOption.xSNMP.xOID.nLen) == 0)
			{
				TRACE("Can't find MIB\n");
				continue;
			}

			FTM_INT	nIndex;
			nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 3], 0, 16);
			pEP->xOption.xSNMP.xOID.pIDs[pEP->xOption.xSNMP.xOID.nLen++] = nIndex & 0xFF;
			FTM_LIST_append(&pNode->xCommon.xEPList, pEP);
		}
	}
	pES7 = (FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_VIRTUAL_FTE_ES7_DATA));
	if (pES7 == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pthread_create(&pES7->xThread, NULL, FTOM_NODE_VIRTUAL_FTE_ES7_process, pES7);

	pNode->pData = pES7;
	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_final
(
	FTOM_NODE_VIRTUAL_PTR pNode
)
{
	ASSERT(pNode != NULL);
	FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR	pES7 = (FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR)pNode->pData;
	
	if (pNode->pData != NULL)
	{
		pES7->bStop = FTM_TRUE;

		pthread_join(pES7->xThread, NULL);

		FTM_MEM_free(pNode->pData);
		pNode->pData = NULL;
	}

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_LOCK_destroy(&pNode->pLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_getEPData
(
	FTOM_NODE_VIRTUAL_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR pES7 = (FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR)pNode->pData;

	pData->ulTime = time(NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	switch(pEP->xInfo.xType)
	{
	case	FTM_EP_TYPE_DI:
		{
			if (pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 1] == '1')
			{
				FTM_VALUE_initINT(&pData->xValue, pES7->bDI); 
			}
		}
		break;

	case	FTM_EP_TYPE_DO:
		{
			if (pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 1] == '1')
			{
				FTM_VALUE_initINT(&pData->xValue, pES7->bDO); 
			}
		}
		break;

	case	FTM_EP_TYPE_TEMPERATURE:
		{
			FTM_ULONG	ulIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 1], 0, 16);
			if (ulIndex < 11)
			{
				FTM_VALUE_initFLOAT(&pData->xValue, pES7->pTemperatures[ulIndex]); 
			}
		}
		break;

	case	FTM_EP_TYPE_HUMIDITY:
		{
			if (pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 1] == '1')
			{
				FTM_VALUE_initFLOAT(&pData->xValue, pES7->fHumidity); 
			}
		}
		break;
	case	FTM_EP_TYPE_GAS:
		{
			if (pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 1] == '1')
			{
				FTM_VALUE_initULONG(&pData->xValue, pES7->ulCO2); 
			}
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_setEPData
(
	FTOM_NODE_VIRTUAL_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet;
	FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR pES7 = (FTOM_NODE_VIRTUAL_FTE_ES7_DATA_PTR)pNode->pData;

	switch(pEP->xInfo.xType)
	{
	case	FTM_EP_TYPE_DO:
		{
			FTOM_MSG_PTR	pMsg;

			pES7->bDO = pData->xValue.xValue.nValue;

			xRet = FTOM_MSG_EP_createInsertData(pData, 1, &pMsg);
			if (xRet == FTM_RET_OK)
			{
				xRet =FTOM_EP_sendMessage(pEP, pMsg);
				if (xRet != FTM_RET_OK)
				{
					FTOM_MSG_destroy(&pMsg);	
				}
			}
		}
		break;

	default:
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;
		}
	}

	return	FTM_RET_OK;
}

FTOM_NODE_CLASS	xVirtualFTE_ES7 = 
{
	.pModel = "fte-es7",
	.xType		= FTOM_NODE_TYPE_VIRTUAL,
	.fCreate	= (FTOM_NODE_CREATE)FTOM_NODE_VIRTUAL_create,
	.fDestroy	= (FTOM_NODE_DESTROY)FTOM_NODE_VIRTUAL_destroy,
	.fInit		= (FTOM_NODE_INIT)FTOM_NODE_VIRTUAL_FTE_ES7_init,
	.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_VIRTUAL_FTE_ES7_final,
	.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_VIRTUAL_FTE_ES7_getEPData,
	.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_VIRTUAL_FTE_ES7_setEPData,
};

