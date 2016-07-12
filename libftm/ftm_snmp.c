#include "ftm_snmp.h"
#include "ftm_trace.h"

FTM_CHAR_PTR	FTM_SNMP_OID_toStr(FTM_SNMP_OID_PTR pOID)
{
	ASSERT(pOID != NULL);

	static FTM_CHAR	pString[1024];
	FTM_ULONG	ulLen = 0;
	FTM_INT		i;
	
	pString[0] = '\0';
	pString[sizeof(pString) - 1] = '\0';

	for(i = 0 ; i < pOID->nLen ; i++)
	{
		ulLen += snprintf(&pString[ulLen], sizeof(pString) - ulLen - 1, ".%lu", pOID->pIDs[i]);
	}

	return	pString;
}

FTM_CHAR_PTR	FTM_SNMP_versionString(FTM_ULONG ulVersion)
{
	switch(ulVersion)
	{
	case	FTM_SNMP_VERSION_1:
		return	"SNMPv1";
	case	FTM_SNMP_VERSION_2:
		return	"SNMPv2";
	case	FTM_SNMP_VERSION_3:
		return	"SNMPv3";
	}

	return	"SNMPv?";
}
