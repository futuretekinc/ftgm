#include <string.h>
#include <time.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftdm_shell_cmds.h"
#include "ftdm_node_info.h"
#include "ftdm_ep_info.h"
#include "ftdm_ep_data.h"
#include "ftdm_config.h"

FTM_RET	FTDM_SHELL_showNodeList(void)
{
	FTM_RET	xRet;
	FTM_ULONG	i, ulCount = 0;

	xRet = FTDM_NODE_INFO_count(&ulCount);
	
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%-16s %-16s %-16s %-8s %-8s %-16s %-16s %-16s %-16s\n", "DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");

		for(i = 0 ; i < ulCount; i++)
		{
			FTM_NODE_INFO_PTR	pInfo = NULL;

			xRet = FTDM_NODE_INFO_getAt(i, &pInfo);
			if (xRet == FTM_RET_OK)
			{
				MESSAGE("%-16s %-16s %-16s %8d %8d ", 
					pInfo->pDID, 
					FTM_nodeTypeString(pInfo->xType), 
				pInfo->pLocation,
				pInfo->ulInterval,
				pInfo->ulTimeout);

				switch(pInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					{
						MESSAGE("%-16s %-16s %-16s", 
							FTDM_CFG_SNMP_getVersionString(pInfo->xOption.xSNMP.ulVersion),
							pInfo->xOption.xSNMP.pURL,
							pInfo->xOption.xSNMP.pCommunity);
					
					}
					break;
				}

				MESSAGE("\n");
			}
		}
	
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_SHELL_showNodeInfo(FTM_CHAR_PTR pDID)
{
	FTM_RET	xRet;
	FTM_NODE_INFO_PTR	pNodeInfo = NULL;

	xRet = FTDM_NODE_INFO_get(pDID, &pNodeInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Node[%s] not found.\n", pDID);
	}
	else
	{
		FTM_ULONG	ulCount = 0;

		MESSAGE("%-16s : %s\n", "DID", 		pDID);	
		MESSAGE("%-16s : %s\n", "TYPE", 	FTM_nodeTypeString(pNodeInfo->xType)); 
		MESSAGE("%-16s : %s\n", "LOCATION", pNodeInfo->pLocation);
		MESSAGE("%-16s : %lu\n", "INTERVAL",pNodeInfo->ulInterval);
		MESSAGE("%-16s : %lu\n", "TIMEOUT", pNodeInfo->ulTimeout);
		MESSAGE("%-16s : %s\n", "OPT 0", 	FTDM_CFG_SNMP_getVersionString(pNodeInfo->xOption.xSNMP.ulVersion));	
		MESSAGE("%-16s : %s\n", "OPT 1", 	pNodeInfo->xOption.xSNMP.pURL);	
		MESSAGE("%-16s : %s\n", "OPT 2", 	pNodeInfo->xOption.xSNMP.pCommunity);	

		if (FTDM_EP_INFO_count(0, &ulCount) == FTM_RET_OK)
		{
			FTM_ULONG	i, ulIndex = 0, ulEPCount = 0;

			for(i = 0 ; i < ulCount ; i++)
			{
				FTM_EP_INFO_PTR	pEPInfo;
		
				xRet = FTDM_EP_INFO_getAt(i, &pEPInfo);
				if (xRet == FTM_RET_OK)
				{
					if (strcasecmp(pEPInfo->pDID, pDID) == 0)
					{
						ulEPCount++;	
					}
				}
			}

			MESSAGE("%-16s : %d\n", "EPs", 	ulEPCount);

			if (ulEPCount != 0)
			{
				MESSAGE("%16s   %-5s %-8s %-16s %-16s %-8s %-8s %-8s %-8s %-16s %-8s\n",
					"", "INDEX", "EPID", "TYPE", "NAME", "UNIT", "STATE", "INTERVAL", "TIMEOUT", "DID", "DEPID");
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_EP_INFO_PTR	pEPInfo;
			
					xRet = FTDM_EP_INFO_getAt(i, &pEPInfo);
					if (xRet == FTM_RET_OK)
					{
						if (strcasecmp(pEPInfo->pDID, pDID) == 0)
						{
							MESSAGE("%16s   %-5d %08lx %-16s %-16s %-8s ",
								"",
								++ulIndex,
								pEPInfo->xEPID,
								FTDM_CFG_EP_getTypeString(pEPInfo->xType),
								pEPInfo->pName,
								pEPInfo->pUnit);
			
							switch(pEPInfo->xState)
							{
							case	FTM_EP_STATE_DISABLE: 	MESSAGE("%-8s ", "DISABLE");  	break; 
							case	FTM_EP_STATE_RUN: 		MESSAGE("%-8s ", "RUN"); 		break; 
							case	FTM_EP_STATE_STOP: 		MESSAGE("%-8s ", "STOP"); 		break;
							case	FTM_EP_STATE_ERROR: 	MESSAGE("%-8s ", "ERROR"); 		break;
							default: MESSAGE("%-8s ", "UNKNOWN");
							}
		
							MESSAGE("%-8lu %-8lu %-16s %08lx\n",
								pEPInfo->ulInterval,
								pEPInfo->ulTimeout,
								pEPInfo->pDID,
								pEPInfo->xDEPID);
						}
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_showEPList(void)
{
	FTM_ULONG	ulCount;

	MESSAGE("# PRE-REGISTERED ENDPOINT\n");
	MESSAGE("%-5s %-8s %-16s %-16s %-8s %-8s %-8s %-8s %-16s %-8s\n",
			"INDEX", "EPID", "TYPE", "NAME", "UNIT", "STATE", "INTERVAL", "TIMEOUT", "DID", "DEPID");
	if (FTDM_EP_INFO_count(0, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_INFO_PTR	pEPInfo;
		
			FTDM_EP_INFO_getAt(i, &pEPInfo);
			MESSAGE("%5d %08lx %-16s %-16s %-8s ",
				i+1,
				pEPInfo->xEPID,
				FTDM_CFG_EP_getTypeString(pEPInfo->xType),
				pEPInfo->pName,
				pEPInfo->pUnit);
		
			switch(pEPInfo->xState)
			{
			case	FTM_EP_STATE_DISABLE: 	MESSAGE("%-8s ", "DISABLE");  	break; 
			case	FTM_EP_STATE_RUN: 		MESSAGE("%-8s ", "RUN"); 		break; 
			case	FTM_EP_STATE_STOP: 		MESSAGE("%-8s ", "STOP"); 		break;
			case	FTM_EP_STATE_ERROR: 	MESSAGE("%-8s ", "ERROR"); 		break;
			default: MESSAGE("%-8s ", "UNKNOWN");
			}
	
			MESSAGE("%-8lu %-8lu %-16s %08lx\n",
				pEPInfo->ulInterval,
				pEPInfo->ulTimeout,
				pEPInfo->pDID,
				pEPInfo->xDEPID);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_showEPInfo(FTM_EPID xEPID)
{
	FTM_RET			xRet;
	FTM_EP_INFO_PTR	pEPInfo = NULL;
	FTM_ULONG		ulDataCount = 0;

	if (FTDM_EP_INFO_get(xEPID, &pEPInfo) != FTM_RET_OK)
	{
		MESSAGE("Invalid EPID [%08x]\n", xEPID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	MESSAGE("%-16s : %08x\n", 	"EPID", pEPInfo->xEPID);
	MESSAGE("%-16s : %s\n", 	"TYPE", FTDM_CFG_EP_getTypeString(pEPInfo->xType));
	MESSAGE("%-16s : %s\n", 	"NAME", pEPInfo->pName);
	MESSAGE("%-16s : %s\n", 	"UNIT", pEPInfo->pUnit);
	MESSAGE("%-16s : ", 		"STATE");

	switch(pEPInfo->xState)
	{
	case	FTM_EP_STATE_DISABLE: 	MESSAGE("%-8s\n", "DISABLE");  	break; 
	case	FTM_EP_STATE_RUN: 		MESSAGE("%-8s\n", "RUN"); 		break; 
	case	FTM_EP_STATE_STOP: 		MESSAGE("%-8s\n", "STOP"); 		break;
	case	FTM_EP_STATE_ERROR: 	MESSAGE("%-8s\n", "ERROR"); 	break;
	default: MESSAGE("%-8s\n", "UNKNOWN");
	}

	MESSAGE("%-16s : %lu\n", 	"INTERVAL", pEPInfo->ulInterval);
	MESSAGE("%-16s : %lu\n", 	"TIMEOUT", 	pEPInfo->ulTimeout);
	MESSAGE("%-16s : %s\n", 	"DID", 		pEPInfo->pDID);
	MESSAGE("%-16s : %08x\n", 	"DEPID", 	pEPInfo->xDEPID);
	MESSAGE("%-16s : %s\n", 	"PID", 		pEPInfo->pPID);
	MESSAGE("%-16s : %08x\n", 	"PEPID", 	pEPInfo->xPEPID);

	xRet = FTDM_EP_DATA_count(xEPID, &ulDataCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%-16s : %d\n", "DATA COUNT", ulDataCount);
	}
	else
	{
		MESSAGE("%-16s : %s\n", "DATA COUNT", "UNKNOWN");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_showEPData(FTM_EPID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount)
{
	FTM_RET			xRet;
	FTM_ULONG		i, ulTotalCount = 0;
	FTM_EP_DATA_PTR pData;

	xRet = FTDM_EP_DATA_count(xEPID, &ulTotalCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("%08x is not exists.\n", xEPID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	if (ulTotalCount <= ulBegin) 
	{
		return	FTM_RET_OK;
	}

	if (ulCount > (ulTotalCount - ulBegin))
	{
		ulCount = ulTotalCount - ulBegin;
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
	if (pData == NULL)
	{
		ERROR("System is not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTDM_EP_DATA_get(xEPID, ulBegin, pData, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pData);
		return	xRet;
	}

	for(i = 0 ; i < ulCount; i++)
	{
		FTM_CHAR	pTime[64];

		strcpy(pTime, ctime((time_t *)&pData[i].ulTime));
		pTime[strlen(pTime) - 1] = '\0';
		MESSAGE("%4d : %16s %10d %d\n", ulBegin + i + 1, pTime, pData[i].xState, pData[i].xValue.ulValue);
	}

	FTM_MEM_free(pData);

	return	FTM_RET_OK;
}

