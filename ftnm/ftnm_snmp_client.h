#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "ftm_types.h"
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
	FTM_ULONG				nVersion;
	FTM_CHAR_PTR			pPeerName;
	FTM_CHAR_PTR			pCommunity;	

	list_t					xOIDList;	
	FTM_INT					nCurrentOID;
	FTM_ULONG				nStatus;
	FTM_BOOL				bCompleted;
	struct snmp_session 	*pSession;		/* SNMP session data */
} FTNM_SNMP_INFO, _PTR_ FTNM_SNMP_INFO_PTR;

FTM_RET	FTNM_snmpClientInit(void);
FTM_RET FTNM_snmpClientFinal(void);

FTM_RET FTNM_snmpClientAsyncCall(FTNM_SNMP_INFO_PTR pInfo);
FTM_BOOL	FTNM_snmpIsRunning(FTNM_SNMP_INFO_PTR pInfo);
FTM_BOOL	FTNM_snmpIsCompleted(FTNM_SNMP_INFO_PTR pInfo);
#endif

