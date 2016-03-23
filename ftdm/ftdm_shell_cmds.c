#include <string.h>
#include <time.h>
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftdm_shell_cmds.h"
#include "ftdm_node.h"
#include "ftdm_ep.h"
#include "ftdm_ep_data.h"
#include "ftdm_config.h"

FTM_RET	FTDM_SHELL_showNodeList(void)
{
	FTM_RET	xRet;
	FTM_ULONG	i, ulCount = 0;

	xRet = FTDM_NODE_count(&ulCount);
	
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%-16s %-16s %-16s %-8s %-8s %-16s %-16s %-16s %-16s\n", "DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");

		for(i = 0 ; i < ulCount; i++)
		{
			FTDM_NODE_PTR	pNode = NULL;

			xRet = FTDM_NODE_getAt(i, &pNode);
			if (xRet == FTM_RET_OK)
			{
				MESSAGE("%-16s %-16s %-16s %8d %8d ", 
					pNode->xInfo.pDID, 
					FTM_NODE_typeString(pNode->xInfo.xType), 
				pNode->xInfo.pLocation,
				pNode->xInfo.ulInterval,
				pNode->xInfo.ulTimeout);

				switch(pNode->xInfo.xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					{
						MESSAGE("%-16s %-16s %-16s", 
							FTDM_CFG_SNMP_getVersionString(pNode->xInfo.xOption.xSNMP.ulVersion),
							pNode->xInfo.xOption.xSNMP.pURL,
							pNode->xInfo.xOption.xSNMP.pCommunity);
					
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
	FTDM_NODE_PTR	pNode = NULL;

	xRet = FTDM_NODE_get(pDID, &pNode);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Node[%s] not found.\n", pDID);
	}
	else
	{
		FTM_ULONG	ulCount = 0;

		MESSAGE("%-16s : %s\n", "DID", 		pDID);	
		MESSAGE("%-16s : %s\n", "TYPE", 	FTM_NODE_typeString(pNode->xInfo.xType)); 
		MESSAGE("%-16s : %s\n", "LOCATION", pNode->xInfo.pLocation);
		MESSAGE("%-16s : %lu\n", "INTERVAL",pNode->xInfo.ulInterval);
		MESSAGE("%-16s : %lu\n", "TIMEOUT", pNode->xInfo.ulTimeout);
		MESSAGE("%-16s : %s\n", "OPT 0", 	FTDM_CFG_SNMP_getVersionString(pNode->xInfo.xOption.xSNMP.ulVersion));	
		MESSAGE("%-16s : %s\n", "OPT 1", 	pNode->xInfo.xOption.xSNMP.pURL);	
		MESSAGE("%-16s : %s\n", "OPT 2", 	pNode->xInfo.xOption.xSNMP.pCommunity);	

		if (FTDM_EP_count(0, &ulCount) == FTM_RET_OK)
		{
			FTM_ULONG	i, ulIndex = 0, ulEPCount = 0;

			for(i = 0 ; i < ulCount ; i++)
			{
				FTDM_EP_PTR	pEP;
		
				xRet = FTDM_EP_getAt(i, &pEP);
				if (xRet == FTM_RET_OK)
				{
					if (strcasecmp(pEP->xInfo.pDID, pDID) == 0)
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
					FTDM_EP_PTR	pEP;
			
					xRet = FTDM_EP_getAt(i, &pEP);
					if (xRet == FTM_RET_OK)
					{
						if (strcasecmp(pEP->xInfo.pDID, pDID) == 0)
						{
							MESSAGE("%16s   %-5d %08lx %-16s %-16s %-8s ",
								"",
								++ulIndex,
								pEP->xInfo.xEPID,
								FTM_EP_typeString(pEP->xInfo.xType),
								pEP->xInfo.pName,
								pEP->xInfo.pUnit);
		
							if(pEP->xInfo.bEnable)
							{
								MESSAGE("%-8s ", "ENABLE");
							}
							else
							{
								MESSAGE("%-8s ", "DISABLE");
							}
		
							MESSAGE("%-8lu %-8lu %-16s %08lx\n",
								pEP->xInfo.ulInterval,
								pEP->xInfo.ulTimeout,
								pEP->xInfo.pDID,
								pEP->xInfo.xDEPID);
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
	if (FTDM_EP_count(0, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTDM_EP_PTR	pEP;
		
			FTDM_EP_getAt(i, &pEP);
			MESSAGE("%5d %08lx %-16s %-16s %-8s ",
				i+1,
				pEP->xInfo.xEPID,
				FTM_EP_typeString(pEP->xInfo.xType),
				pEP->xInfo.pName,
				pEP->xInfo.pUnit);

			if (pEP->xInfo.bEnable)
			{
				MESSAGE("%-8s ", "ENABLE");
			}
			else
			{
				MESSAGE("%-8s ", "DISABLE");
			}
	
			MESSAGE("%-8lu %-8lu %-16s %08lx\n",
				pEP->xInfo.ulInterval,
				pEP->xInfo.ulTimeout,
				pEP->xInfo.pDID,
				pEP->xInfo.xDEPID);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_showEPInfo(FTM_EP_ID xEPID)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP = NULL;
	FTM_ULONG	ulDataCount = 0;

	if (FTDM_EP_get(xEPID, &pEP) != FTM_RET_OK)
	{
		MESSAGE("Invalid EPID [%08x]\n", xEPID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	MESSAGE("%-16s : %08x\n", 	"EPID", pEP->xInfo.xEPID);
	MESSAGE("%-16s : %s\n", 	"TYPE", FTM_EP_typeString(pEP->xInfo.xType));
	MESSAGE("%-16s : %s\n", 	"NAME", pEP->xInfo.pName);
	MESSAGE("%-16s : %s\n", 	"UNIT", pEP->xInfo.pUnit);
	MESSAGE("%-16s : ", 		"STATE");

	if (pEP->xInfo.bEnable)
	{
		MESSAGE("%-8s\n", "ENABLE");
	}
	else
	{
		MESSAGE("%-8s\n", "DISABLE");
	}

	MESSAGE("%-16s : %lu\n", 	"INTERVAL", pEP->xInfo.ulInterval);
	MESSAGE("%-16s : %lu\n", 	"TIMEOUT", 	pEP->xInfo.ulTimeout);
	MESSAGE("%-16s : %s\n", 	"DID", 		pEP->xInfo.pDID);
	MESSAGE("%-16s : %08x\n", 	"DEPID", 	pEP->xInfo.xDEPID);
	MESSAGE("%-16s : %s\n", 	"PID", 		pEP->xInfo.pPID);
	MESSAGE("%-16s : %08x\n", 	"PEPID", 	pEP->xInfo.xPEPID);

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

FTM_RET	FTDM_SHELL_showEPData(FTM_EP_ID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount)
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

			MESSAGE("%4d : %16s ", ulBegin + i + 1, pTime);
		switch(pData[i].xState)
		{
		case	FTM_EP_DATA_STATE_VALID:
			{
				MESSAGE("%10s ", "VALID"); 
			}
			break;

		case	FTM_EP_DATA_STATE_INVALID:
			{
				MESSAGE("%10s ", "INVALID"); 
			}
			break;

		default:
			{
				MESSAGE("%10s ", "UNKNOWN"); 
			}
		}

		switch(pData[i].xType)
		{
		case	FTM_EP_DATA_TYPE_INT:
			MESSAGE("%d\n", pData[i].xValue.nValue);
			break;

		case	FTM_EP_DATA_TYPE_ULONG:
			MESSAGE("%lu\n", pData[i].xValue.ulValue);
			break;

		case	FTM_EP_DATA_TYPE_FLOAT:
			MESSAGE("%5.2f\n", pData[i].xValue.fValue);
			break;
		}
	}

	FTM_MEM_free(pData);

	return	FTM_RET_OK;
}

