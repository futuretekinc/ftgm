#include "ftm_snmp.h"

FTM_CHAR_PTR	FTM_SNMP_OID_toStr(FTM_SNMP_OID_PTR pOID)
{
	ASSERT(pOID != NULL);

	static FTM_CHAR	pString[1024];
	FTM_ULONG	ulLen = 0;
	FTM_INT		i;
	
	pString[0] = '\0';
	pString[sizeof(pString) - 1] = '\0';

	for(i = 0 ; i < pOID->ulOIDLen ; i++)
	{
		ulLen += snprintf(&pString[ulLen], sizeof(pString) - ulLen - 1, ".%lu", pOID->pOID[i]);
	}

	return	pString;
}
