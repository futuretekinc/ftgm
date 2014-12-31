#include <string.h>
#include <stdio.h>
#include "ftnm.h"
#include "ftdm_type.h"
#include "ftnm_config.h"
#include "ftnm_snmp_client.h"

FTDM_DEVICE_INFO	xDeviceInfo[] = 
{
	{"0001", 1, "www.0001.com", "0.0.0.1"},
	{"0002", 2, "www.0002.com", "0.0.0.2"},
	{"0003", 3, "www.0003.com", "0.0.0.3"},
	{"0004", 4, "www.0004.com", "0.0.0.4"}
};

FTNM_SNMP_OID	xOIDsysDescr =
{
	.pOID = {1,3,6,1,2,1,1,1,0},
	.nOIDLen = 9,
};

FTNM_SNMP_INFO	xSNMPInfo =	
{
	.nVersion = SNMP_VERSION_2c,
	.pPeerName = "10.0.1.35",
	.pCommunity = "public"
};

int main(int argc, char *argv[])
{
	FTM_RET	nRet;
	FTM_INT	i;

	setPrintMode(2);

	nRet = FTNM_configInit();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_configInit failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_configLoad("ftnm.conf");
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_configLoad failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_snmpClientInit();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_snmpClientInit failed [ nRet = %08lx]\n", nRet);
		return	nRet;	
	}

	nRet = FTNM_initNodeManager();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTNM_initNodeManager failed [nRet = %08lx]\n", nRet);
		return	0;	
	}
	MESSAGE("FTNM_initNodeManager initialized\n");

	for(i = 0 ; i < 4 ; i++)
	{
		nRet = FTNM_createNode(&xDeviceInfo[i]); if (nRet != FTM_RET_OK) 
		{
			MESSAGE("FTNM_createNode(%d) failed[nRet = %08lx]\n", i, nRet);
		}
		else
		{
			MESSAGE("FTNM_crateNode(%d) done\n", i);
		}
	}

	
	for(i = 0 ; i < 4 ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_CHAR	pDID[FTDM_DEVICE_ID_LEN+1];

		sprintf(pDID, "%04x", i+1);
		if (FTNM_getNode(pDID, &pNode) == FTM_RET_OK)
		{
			FTNM_startNode(pNode);
		}
	}

	list_init(&xSNMPInfo.xOIDList);
	list_append(&xSNMPInfo.xOIDList, &xOIDsysDescr);
	FTNM_snmpClientAsyncCall(&xSNMPInfo);

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
