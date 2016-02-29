#ifndef	_FTNM_SNMPTRAPD_H_
#define	_FTNM_SNMPTRAPD_H_

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ftm.h"
#include "ftnm_snmp.h"

#define	FTNM_SNMPTRAPD_NAME_LENGTH	128
#define	FTNM_SNMPTRAPD_NAME			"ftnm:snmptrapd"
#define	FTNM_SNMPTRAPD_PORT			162

typedef	enum
{
	FTNM_SNMPTRAPD_MSG_TYPE_UNKNOWN = 0,
	FTNM_SNMPTRAPD_MSG_TYPE_EP_CHANGED,
} FTNM_SNMPTRAPD_MSG_TYPE, _PTR_ FTNM_SNMPTRAPD_MSG_TYPE_PTR;

typedef	FTM_RET (*FTNM_SNMPTRAPD_CALLBACK)(FTM_CHAR_PTR pTrapMsg);

typedef struct
{
	FTM_CHAR				pName[FTNM_SNMPTRAPD_NAME_LENGTH + 1];
	FTM_USHORT				usPort;
}	FTNM_SNMPTRAPD_CONFIG, _PTR_ FTNM_SNMPTRAPD_CONFIG_PTR;
	
typedef	struct
{
	FTNM_SNMPTRAPD_CONFIG	xConfig;
	FTNM_SNMPTRAPD_CALLBACK	fTrapCB;
	FTM_BOOL				bRunning;
	pthread_t				xPThread;
	netsnmp_transport 		*pTransport;
}	FTNM_SNMPTRAPD, _PTR_ FTNM_SNMPTRAPD_PTR;

FTM_RET	FTNM_SNMPTRAPD_init(FTNM_SNMPTRAPD_PTR pCTX);
FTM_RET	FTNM_SNMPTRAPD_final(FTNM_SNMPTRAPD_PTR pCTX);

FTM_RET FTNM_SNMPTRAPD_start(FTNM_SNMPTRAPD_PTR pCTX);
FTM_RET FTNM_SNMPTRAPD_stop(FTNM_SNMPTRAPD_PTR pCTX);

FTM_RET FTNM_SNMPTRAPD_loadFromFile(FTNM_SNMPTRAPD_PTR pCTX, FTM_CHAR_PTR pFileName);
FTM_RET FTNM_SNMPTRAPD_showConfig(FTNM_SNMPTRAPD_PTR pCTX);

FTM_RET	FTNM_SNMPTRAPD_setCallback(FTNM_SNMPTRAPD_PTR pCTX, FTNM_SNMPTRAPD_CALLBACK fTrapCB);
FTM_RET	FTNM_SNMPTRAPD_addTrapOID(FTNM_SNMPTRAPD_PTR pCTX, FTNM_SNMP_OID_PTR pOID);
#endif
