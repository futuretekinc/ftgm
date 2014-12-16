#include "libftdm.h"

FTGM_RET 	FTDM_init(void)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_final(void)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_devInsert
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_DEVICE_TYPE 	xType, 
	FTGM_STRING 		strURL, 
	FTGM_STRING 		strLocation
)
{
	return	FTGM_RET_OK;
}

FTGM_RET 	FTDM_devRemove
(
	FTGM_DEVICE_ID 			xDID
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_devInfo
(
	FTGM_DEVICE_ID 			xDID, 
	FTGM_DEVICE_INFO_PTR 	pInfo
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_devType
(
	FTGM_DEVICE_ID 			xDID, 
	FTGM_DEVICE_TYPE_PTR 	pType
)
{
	return	FTGM_RET_OK;
}

FTGM_RET 	FTDM_devURL
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff, 
	FTGM_INT 			nBuffLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET 	FTDM_devURLSet
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff
)
{
	return	FTGM_RET_OK;
}

FTGM_RET 	FTDM_devLocation
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff, 
	FTGM_INT 			nBuffLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET 	FTDM_devLocationSet
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STING 			strBuff
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_epInsert
(
	FTGM_EP_ID 			xEPID, 
	FTGM_EP_INFO_PTR 	pInfo
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_epRemove
(
	FTGM_EP_ID 			xEPID
)
{
	return	FTGM_RET_OK;
}

FTGM_REG	FTDM_epInfo
(
	FTGM_EP_ID			xEPID,
	FTGM_EP_INFO_PTR	pInfo
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_epDataAppend
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG			nTime, 
	FTGM_ULONG 			nValue
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_epData
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG 			nBeginTime, 
	FTGM_ULONG 			nEndTime, 
	FTGM_ULONG_PTR		pnCount, 
	FTGM_EP_DATA_PTR 	pData
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_epDataRemove
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG 			nBeginTime, 
	FTGM_ULONG 			nEndTime, 
	FTGM_ULONG_PTR		nCount
) 
{
	return	FTGM_RET_OK;
}

