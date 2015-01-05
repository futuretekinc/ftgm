#include "ftm.h"

FTM_CHAR_PTR FTM_nodeTypeString(FTM_NODE_TYPE nType)
{
	static FTM_CHAR	pBuff[256];
	switch(nType)
	{
	case	FTM_NODE_TYPE_SNMP: return	"SNMP";
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP: return	"MODBUS/TCP";
	case	FTM_NODE_TYPE_MODBUS_OVER_SERIAL: return	"MODBUS/SERIAL";
	default:	sprintf(pBuff, "UNKNOWN(%08lx)", nType);
	}

	return	pBuff;
}
