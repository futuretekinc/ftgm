#ifndef	__FTOM_SNMP_CLIENT_H__
#define	__FTOM_SNMP_CLIENT_H__

#include "ftom.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_snmptrapd.h"

#define	FTOM_SNMPC_NAME_LENGTH	128
#define	FTOM_SNMPC_NAME			"ftom:agent"
#define	FTOM_SNMPC_RETRY_COUNT	3

typedef	FTM_ULONG		FTOM_SNMPC_STATE, _PTR_ FTOM_SNMPC_STATE_PTR;

#define	FTOM_SNMPC_STATE_UNKNOWN		0x00000000
#define	FTOM_SNMPC_STATE_INITIALIZED	0x00000001
#define	FTOM_SNMPC_STATE_RUNNING		0x00000002
#define	FTOM_SNMPC_STATE_TIMEOUT		0x00000003
#define	FTOM_SNMPC_STATE_ERROR			0x00000004
#define	FTOM_SNMPC_STATE_COMPLETED		0x00000005

typedef	struct	FTOM_STRUCT _PTR_ FTOM_PTR;

typedef	struct
{
	FTM_CHAR		pName[FTOM_SNMPC_NAME_LENGTH + 1];
	FTM_LIST		xMIBList;

	FTM_ULONG		ulMaxRetryCount;
}	FTOM_SNMPC_CONFIG, _PTR_ FTOM_SNMPC_CONFIG_PTR;

typedef	struct 
{
	FTOM_SNMPC_CONFIG		xConfig;
	pthread_t				xPThread;

	FTM_BOOL				bStop;
	FTM_LOCK				xLock;
	FTOM_PTR				pOM;
	FTOM_SERVICE_ID			xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTOM_SNMPC, _PTR_ FTOM_SNMPC_PTR;

FTM_RET	FTOM_SNMPC_init
(
	FTOM_SNMPC_PTR 	pClient,
	FTOM_PTR 			pCTX 
);

FTM_RET	FTOM_SNMPC_final
(
	FTOM_SNMPC_PTR pClient
);

FTM_RET FTOM_SNMPC_loadFromFile
(
	FTOM_SNMPC_PTR pClient, 
	FTM_CHAR_PTR pFileName
);

FTM_RET FTOM_SNMPC_showConfig
(
	FTOM_SNMPC_PTR pClient
);

FTM_RET FTOM_SNMPC_start
(
	FTOM_SNMPC_PTR pClient
);

FTM_RET FTOM_SNMPC_stop
(
	FTOM_SNMPC_PTR pClient
);

FTM_RET	FTOM_SNMPC_setServiceCallback
(
	FTOM_SNMPC_PTR 		pClient, 
	FTOM_SERVICE_ID 	xID, 
	FTOM_SERVICE_CALLBACK fServiceCB
);

FTM_RET	FTOM_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_SNMPC_getOID
(	
	FTM_CHAR_PTR 	pInput, 
	oid 			*pOID, 
	size_t 			*pnOIDLen
);

#endif

