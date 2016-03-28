#ifndef	_FTM_OM_SNMPTRAPD_H_
#define	_FTM_OM_SNMPTRAPD_H_

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ftm.h"
#include "ftm_om_snmp.h"
#include "ftm_om_service.h"

#define	FTM_OM_SNMPTRAPD_NAME_LENGTH	128
#define	FTM_OM_SNMPTRAPD_NAME			"ftm_om:snmptrapd"
#define	FTM_OM_SNMPTRAPD_PORT			162

typedef	enum
{
	FTM_OM_SNMPTRAPD_MSG_TYPE_UNKNOWN = 0,
	FTM_OM_SNMPTRAPD_MSG_TYPE_EP_CHANGED,
} FTM_OM_SNMPTRAPD_MSG_TYPE, _PTR_ FTM_OM_SNMPTRAPD_MSG_TYPE_PTR;

typedef struct
{
	FTM_CHAR				pName[FTM_OM_SNMPTRAPD_NAME_LENGTH + 1];
	FTM_USHORT				usPort;
}	FTM_OM_SNMPTRAPD_CONFIG, _PTR_ FTM_OM_SNMPTRAPD_CONFIG_PTR;

typedef	FTM_RET (*FTM_OM_SNMPTRAPD_CALLBACK)(FTM_CHAR_PTR pTrapMsg);

typedef	struct
{
	FTM_OM_SNMPTRAPD_CONFIG	xConfig;
	FTM_LIST				xTrapCBList;
	FTM_BOOL				bStop;
	pthread_t				xPThread;
	netsnmp_transport 		*pTransport;

	FTM_OM_PTR		pOM;
	FTM_OM_SERVICE_ID			xServiceID;
	FTM_OM_SERVICE_CALLBACK	fServiceCB;
}	FTM_OM_SNMPTRAPD, _PTR_ FTM_OM_SNMPTRAPD_PTR;

FTM_RET	FTM_OM_SNMPTRAPD_init
(
	FTM_OM_PTR pOM, 
	FTM_OM_SNMPTRAPD_PTR pSNMPTRAPD
);

FTM_RET	FTM_OM_SNMPTRAPD_final
(
	FTM_OM_SNMPTRAPD_PTR pSNMPTRAPD
);

FTM_RET FTM_OM_SNMPTRAPD_start
(
	FTM_OM_SNMPTRAPD_PTR pSNMPTRAPD
);

FTM_RET FTM_OM_SNMPTRAPD_stop
(
	FTM_OM_SNMPTRAPD_PTR pSNMPTRAPD
);

FTM_RET FTM_OM_SNMPTRAPD_loadFromFile
(
	FTM_OM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET FTM_OM_SNMPTRAPD_showConfig
(
	FTM_OM_SNMPTRAPD_PTR pSNMPTRAPD
);

FTM_RET	FTM_OM_SNMPTRAPD_setServiceCallback
(
	FTM_OM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_OM_SERVICE_ID 		xID, 
	FTM_OM_SERVICE_CALLBACK fServiceCB
);

FTM_RET	FTM_OM_SNMPTRAPD_addTrapOID
(
	FTM_OM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_OM_SNMP_OID_PTR 	pOID
);
#endif
