#ifndef	_FTM_OM_SNMP_H_
#define	_FTM_OM_SNMP_H_

#include <net-snmp/library/oid.h>
#include "ftm.h"

#define	FTM_OM_SNMP_OID_LENGTH	32

typedef	struct
{
	oid			pOID[FTM_OM_SNMP_OID_LENGTH];
	FTM_ULONG	ulOIDLen;
}	FTM_OM_SNMP_OID, _PTR_ FTM_OM_SNMP_OID_PTR;	


#endif
