#ifndef	__FTM_OM_SNMP_CLIENT_H__
#define	__FTM_OM_SNMP_CLIENT_H__

#include "ftm_om.h"
#include "ftm_om_node.h"
#include "ftm_om_ep.h"
#include "ftm_om_snmptrapd.h"

#define	FTM_OM_SNMPC_NAME_LENGTH	128
#define	FTM_OM_SNMPC_NAME			"ftm_om:agent"
#define	FTM_OM_SNMPC_RETRY_COUNT	3

typedef	FTM_ULONG		FTM_OM_SNMPC_STATE, _PTR_ FTM_OM_SNMPC_STATE_PTR;

#define	FTM_OM_SNMPC_STATE_UNKNOWN		0x00000000
#define	FTM_OM_SNMPC_STATE_INITIALIZED	0x00000001
#define	FTM_OM_SNMPC_STATE_RUNNING		0x00000002
#define	FTM_OM_SNMPC_STATE_TIMEOUT		0x00000003
#define	FTM_OM_SNMPC_STATE_ERROR			0x00000004
#define	FTM_OM_SNMPC_STATE_COMPLETED		0x00000005

typedef	struct	FTM_OM_STRUCT _PTR_ FTM_OM_PTR;

typedef	struct
{
	FTM_CHAR		pName[FTM_OM_SNMPC_NAME_LENGTH + 1];
	FTM_LIST		xMIBList;

	FTM_ULONG		ulMaxRetryCount;
}	FTM_OM_SNMPC_CONFIG, _PTR_ FTM_OM_SNMPC_CONFIG_PTR;

typedef	struct 
{
	FTM_OM_SNMPC_CONFIG		xConfig;
	pthread_t				xPThread;
	FTM_BOOL				bStop;

	FTM_OM_PTR				pOM;
	FTM_OM_SERVICE_ID		xServiceID;
	FTM_OM_SERVICE_CALLBACK	fServiceCB;
}	FTM_OM_SNMPC, _PTR_ FTM_OM_SNMPC_PTR;

FTM_RET	FTM_OM_SNMPC_init
(
	FTM_OM_SNMPC_PTR 	pClient,
	FTM_OM_PTR 			pCTX 
);

FTM_RET	FTM_OM_SNMPC_final
(
	FTM_OM_SNMPC_PTR pClient
);

FTM_RET FTM_OM_SNMPC_loadFromFile
(
	FTM_OM_SNMPC_PTR pClient, 
	FTM_CHAR_PTR pFileName
);

FTM_RET FTM_OM_SNMPC_showConfig
(
	FTM_OM_SNMPC_PTR pClient
);

FTM_RET FTM_OM_SNMPC_start
(
	FTM_OM_SNMPC_PTR pClient
);

FTM_RET FTM_OM_SNMPC_stop
(
	FTM_OM_SNMPC_PTR pClient
);

FTM_RET	FTM_OM_SNMPC_setServiceCallback
(
	FTM_OM_SNMPC_PTR pClient, 
	FTM_OM_SERVICE_ID xID, 
	FTM_OM_SERVICE_CALLBACK fServiceCB
);

FTM_RET	FTM_OM_SNMPC_getEPData
(
	FTM_OM_NODE_SNMPC_PTR pNode, 
	FTM_OM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTM_OM_SNMPC_setEPData
(
	FTM_OM_NODE_SNMPC_PTR pNode, 
	FTM_OM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTM_OM_SNMPC_getOID
(	
	FTM_CHAR_PTR 	pInput, 
	oid 			*pOID, 
	size_t 			*pnOIDLen
);

#endif

