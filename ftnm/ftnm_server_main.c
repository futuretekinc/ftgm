#include <string.h>
#include <stdio.h>
#include "ftnm.h"
#include "ftdm_type.h"
#include "ftnm_config.h"
#include "ftnm_snmp_client.h"

#define	FTNM_TEST_NODE_COUNT	10

void 	FTNM_TEST_nodeInit(void);

FTM_NODE_INFO	xNodeInfo[FTNM_TEST_NODE_COUNT]; 

FTNM_SNMP_OID	xOIDsysDescr =
{
	.pOID = {1,3,6,1,2,1,1,1,0},
	.nOIDLen = 9,
};

int main(int argc, char *argv[])
{
	FTM_RET	nRet;
	FTM_INT	i;

	setPrintMode(2);

	nRet = FTNM_configInit();
	if (nRet != FTM_RET_OK)
	{
		TRACE("FTNM_configInit failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_configLoad("ftnm.conf");
	if (nRet != FTM_RET_OK)
	{
		TRACE("FTNM_configLoad failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_snmpClientInit();
	if (nRet != FTM_RET_OK)
	{
		TRACE("FTNM_snmpClientInit failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_initNodeManager();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_initNodeManager failed [nRet = %08lx]\n", nRet);
		return	0;	
	}
	MESSAGE("FTNM_initNodeManager initialized\n");

	FTNM_TEST_nodeInit();


	for(i = 0 ; i < FTNM_TEST_NODE_COUNT ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_CHAR	pDID[FTM_DID_LEN+1];

		sprintf(pDID, "%04x", i+1);
		if (FTNM_getNode(pDID, &pNode) == FTM_RET_OK)
		{
			TRACE("pNode = %08lx\n", pNode);
			FTNM_startNode(pNode);
		}
		else
		{
			MESSAGE("Node(%s) not found!\n", pDID);
		}
	}

	for(i = 0 ; i < 4 ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_CHAR	pDID[FTM_DID_LEN+1];

		sprintf(pDID, "%04x", i+1);
		if (FTNM_getNode(pDID, &pNode) == FTM_RET_OK)
		{
			TRACE("Waiting for thread(%08lx) termination.\n", pNode->xPThread);
			pthread_join(pNode->xPThread, NULL);	
		}
	}

	MESSAGE("Call FTNM_finalNodeManager\n");
	nRet = FTNM_finalNodeManager();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_finalNodeManager failed [nRet = %08lx]\n", nRet);
	}

	nRet = FTNM_snmpClientFinal();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_snmpClientFinal failed [nRet = %08lx]\n", nRet);
	}

	nRet = FTNM_configFinal();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_configFinal failed [nRet = %08lx]\n", nRet);
	}


	return	0;
}

void 	FTNM_TEST_nodeInit(void)
{
	FTM_INT	i;
	FTM_RET	nRet;

	for(i = 0 ; i < FTNM_TEST_NODE_COUNT ; i++)
	{
		FTNM_NODE_PTR pNode = NULL;

		snprintf(xNodeInfo[i].pDID, FTM_DID_LEN, "%04x", i+1);
		xNodeInfo[i].xType = 0;
		snprintf(xNodeInfo[i].xOption.xSNMP.pURL, FTM_URL_LEN, "10.0.1.100");
		nRet = FTNM_createNodeSNMP(&xNodeInfo[i], &pNode); 
		if (nRet != FTM_RET_OK) 
		{
			TRACE("FTNM_createNodeSNMP failed[nRet = %08lx]\n", nRet);
		}
		else
		{
			TRACE("pNode = %08lx, pNode->pDID = %s\n", 
				pNode, pNode->xInfo.pDID);
		}
	}
}
