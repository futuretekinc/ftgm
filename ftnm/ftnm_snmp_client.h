#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "ftm_types.h"
#include "ftnm_node.h"
#include "simclist.h"

#define	FTNM_SNMP_STATUS_RUNNING	0x00000001
#define	FTNM_SNMP_STATUS_COMPLETED	0x00000010
typedef struct
{
	oid						pOID[MAX_OID_LEN];
	FTM_INT					nOIDLen;
	time_t					xTime;
	FTM_BYTE_PTR			pValue;
	FTM_INT					nValueLen;
}	FTNM_SNMP_OID, _PTR_ FTNM_SNMP_OID_PTR;

typedef	struct
{
	list_t					xCallList;	
	FTM_ULONG				nStatus;

	struct snmp_session 	*pSession;		/* SNMP session data */
}	FTNM_SNMP_CONTEXT, _PTR_ FTNM_SNMP_CONTEXT_PTR;

FTM_RET		FTNM_snmpClientInit(void);
FTM_RET 	FTNM_snmpClientFinal(void);

FTM_RET 	FTNM_snmpClientAsyncCall(FTNM_NODE_PTR pNode);
FTM_BOOL	FTNM_snmpIsRunning(FTNM_NODE_PTR pNode);
FTM_BOOL	FTNM_snmpIsCompleted(FTNM_NODE_PTR pNode);
#endif

