#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_virtual.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_init
(
	FTOM_NODE_VIRTUAL_PTR pNode
)
{
	FTM_RET				xRet;
	FTM_ULONG			ulEPCount;

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
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->xOption.xSNMP.pOID, &pEP->xOption.xSNMP.nOIDLen) == 0)
			{
				TRACE("Can't find MIB\n");
				continue;
			}

			FTM_INT	nIndex;
			nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 3], 0, 16);
			pEP->xOption.xSNMP.pOID[pEP->xOption.xSNMP.nOIDLen++] = nIndex & 0xFF;
			FTM_LIST_append(&pNode->xCommon.xEPList, pEP);
		}
	}
	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_VIRTUAL_FTE_ES7_final
(
	FTOM_NODE_VIRTUAL_PTR pNode
)
{
	ASSERT(pNode != NULL);

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
	pData->ulTime = time(NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	switch(pEP->xInfo.xType)
	{
	case	FTM_EP_TYPE_DI:
	case	FTM_EP_TYPE_DO:
		{
			static	FTM_INT	nValue = 0;
			static	FTM_BOOL	bUp = FTM_FALSE;
			if (bUp)
			{
				if (nValue < 100)
				{
					nValue ++;	
				}
				else
				{
					bUp = FTM_FALSE;
					nValue --;
				}
			}
			else
			{
				if (-nValue > 100)
				{
					nValue --;	
				}
				else
				{
					bUp = FTM_TRUE;
					nValue ++;
				}
			}

			pData->xType = FTM_EP_DATA_TYPE_INT;
			FTM_VALUE_initINT(&pData->xValue, nValue); 
		}
		break;

	case	FTM_EP_TYPE_COUNT:
	case	FTM_EP_TYPE_PRESSURE:
		{
			static	FTM_ULONG	ulValue = 0;
			static	FTM_BOOL	bUp = FTM_FALSE;
			if (bUp)
			{
				if (ulValue < 200)
				{
					ulValue ++;	
				}
				else
				{
					bUp = FTM_FALSE;
					ulValue --;
				}
			}
			else
			{
				if (ulValue > 0)
				{
					ulValue --;	
				}
				else
				{
					bUp = FTM_TRUE;
					ulValue ++;
				}
			}

			pData->xType = FTM_EP_DATA_TYPE_ULONG;
			FTM_VALUE_initULONG(&pData->xValue, ulValue); 
		}
		break;

	case	FTM_EP_TYPE_TEMPERATURE:
	case	FTM_EP_TYPE_HUMIDITY:
	case	FTM_EP_TYPE_VOLTAGE:
	case	FTM_EP_TYPE_CURRENT:
	case	FTM_EP_TYPE_GAS:
	case	FTM_EP_TYPE_POWER:
	case	FTM_EP_TYPE_AI:
		{
			static	FTM_FLOAT	fValue = 0;
			static	FTM_FLOAT	fDelta = 0;
			static	FTM_BOOL	bUp = FTM_FALSE;

			fDelta = rand() % 200 / 100.0;	

			if (bUp)
			{
				if (fValue < 30.0)
				{
					fValue += fDelta;
				}
				else
				{
					bUp = FTM_FALSE;
					fValue -= fDelta;
				}
			}
			else
			{
				if (fValue > 0)
				{
					fValue -= fDelta;
				}
				else
				{
					bUp = FTM_TRUE;
					fValue += fDelta;
				}
			}

			pData->xType = FTM_EP_DATA_TYPE_FLOAT;
			FTM_VALUE_initFLOAT(&pData->xValue, fDelta); 
		}
		break;
#if 0
	case	FTM_EP_DATA_TYPE_BOOL:	
		{
			static	FTM_ULONG	ulValue = 0;
			static	FTM_BOOL	bUp = FTM_FALSE;
			if (bUp)
			{
				if (ulValue < 200)
				{
					ulValue ++;	
				}
				else
				{
					bUp = FTM_FALSE;
					ulValue --;
				}
			}
			else
			{
				if (ulValue > 0)
				{
					ulValue --;	
				}
				else
				{
					bUp = FTM_TRUE;
					ulValue ++;
				}
			}

			pData->xType = FTM_EP_DATA_TYPE_BOOL;
			FTM_VALUE_initBOOL(&pData->xValue, ulValue / 7 % 2); 
		}
		break;
#endif
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

