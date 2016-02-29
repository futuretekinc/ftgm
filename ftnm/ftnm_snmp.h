#ifndef	_FTNM_SNMP_H_
#define	_FTNM_SNMP_H_

#include <net-snmp/library/oid.h>
#include "ftm.h"

#define	FTNM_SNMP_OID_LENGTH	32

typedef	struct
{
	oid			pOID[FTNM_SNMP_OID_LENGTH];
	FTM_ULONG	ulOIDLen;
}	FTNM_SNMP_OID, _PTR_ FTNM_SNMP_OID_PTR;	


#endif
