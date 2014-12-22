#include "ftdm.h"
#include "ftdm_debug.h"

FTDM_VOID	FTDM_dumpPacket
(
	FTDM_CHAR_PTR	pName,
	FTDM_BYTE_PTR	pPacket,
	FTDM_INT		nLen
)
{
	FTDM_INT	i;

	printf("NAME : %s\n", pName);
	for(i = 0 ; i < nLen ; i++)
	{
		printf("%02x ", pPacket[i]);	
		if ((i+1) % 8 == 0)
		{
			printf("\n");	
		}
	}

	if (i % 8 != 0)
	{
		printf("\n");	
	}
}
