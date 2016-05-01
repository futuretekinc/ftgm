#ifndef	_FTM_SNMP_H_
#define	_FTM_SNMP_H_

#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/oid.h>
#include "ftm_types.h"

#define	FTM_SNMP_VERSION_1	1 
#define	FTM_SNMP_VERSION_2	2
#define	FTM_SNMP_VERSION_3	3
	
#define	FTM_SNMP_OID_LENGTH	32

typedef	struct FTM_SNMP_OID_STRUCT
{
	oid			pOID[FTM_SNMP_OID_LENGTH];
	FTM_ULONG	ulOIDLen;
}	FTM_SNMP_OID, _PTR_ FTM_SNMP_OID_PTR;	

FTM_CHAR_PTR	FTM_SNMP_OID_toStr(FTM_SNMP_OID_PTR pOID);
FTM_CHAR_PTR	FTM_SNMP_versionString(FTM_ULONG ulVersion);
#endif
