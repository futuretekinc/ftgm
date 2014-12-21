#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm_device.h"
#include "sqlite_if.h"
#include "debug.h"

static FTDM_DEVICE	_header = 
{
	.pPrev		=	NULL,
	.pNext		=	NULL,
	.xInfo		= 
	{
		.pDID		=	{ 0, },
		.xType		=	FTDM_DT_ROOT,
		.pURL		=	"localhost",
		.pLocation	=	"ROOT"
	}
};

FTDM_RET	FTDM_initDevice
(
 	FTDM_VOID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_insertDevice
(
 	FTDM_DEVICE_PTR pDevice
)
{
	pDevice->pPrev = _header.pPrev;
	pDevice->pNext = &_header;

	_header.pPrev->pNext = pDevice;
	_header.pPrev = pDevice;

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_removeDevice
(
 	FTDM_DEVICE_PTR pDevice
)
{
	pDevice->pPrev->pNext = pDevice->pNext;
	pDevice->pNext->pPrev = pDevice->pPrev;

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getDevice
(
 	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_PTR	_PTR_ 	ppDevice
)
{
	FTDM_DEVICE_PTR	pCurrent = _header.pNext;

	if ((pDID == NULL) || (ppDevice == NULL))
	{
		return	FTDM_RET_INVALID_ARGUMENTS;
	}

	while(pCurrent != &_header)
	{
		if (strcmp(pCurrent->xInfo.pDID, pDID) == 0)
		{
			*ppDevice = pCurrent;	
			return	FTDM_RET_OK;
		}

		pCurrent = pCurrent->pNext;
	}

	return	FTDM_RET_OBJECT_NOT_FOUND;
}

FTDM_RET    FTDM_createDevice
(   
	FTDM_CHAR_PTR       pDID,
	FTDM_DEVICE_TYPE    xType,
	FTDM_CHAR_PTR       pURL,
	FTDM_CHAR_PTR       pLocation
)   
{
	FTDM_RET    		nRet;
	FTDM_DEVICE_PTR		pDevice	;

	if (FTDM_getDevice(pDID, &pDevice) == FTDM_RET_OK)
	{
		return	FTDM_RET_ALREADY_EXIST_OBJECT;
	}

	if ((pURL == NULL) || 
		(strlen(pURL) > FTDM_DEVICE_URL_LEN) ||
		(pLocation == NULL) ||
		(strlen(pLocation) > FTDM_DEVICE_LOCATION_LEN))
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	pDevice = (FTDM_DEVICE_PTR)malloc(sizeof(FTDM_DEVICE));
	if (pDevice == NULL)
	{
		return	FTDM_RET_NOT_ENOUGH_MEMORY;	
	}

	strncpy(pDevice->xInfo.pDID, pDID, FTDM_DEVICE_ID_LEN);
	pDevice->xInfo.xType = xType;
	strcpy(pDevice->xInfo.pURL, pURL);
	strcpy(pDevice->xInfo.pLocation, pLocation);	


	nRet = FTDM_DBIF_insertDeviceInfo(&pDevice->xInfo);
	if (nRet != FTDM_RET_OK)
	{
		free(pDevice);
	}
	else
	{
		FTDM_insertDevice(pDevice);	
	}

	return  nRet;
}	  

FTDM_RET 	FTDM_destroyDevice
(
	FTDM_CHAR_PTR			pDID
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT(pDID != NULL);

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_DBIF_removeDeviceInfo(pDID);
	FTDM_removeDevice(pDevice);
	free(pDevice);

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getDeviceInfo
(
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_INFO_PTR 	pInfo
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL) && (pInfo != NULL));

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	memcpy(pInfo, &pDevice->xInfo, sizeof(FTDM_DEVICE_INFO));

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getDeviceType
(
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_TYPE_PTR 	pType
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL) && (pType != NULL));

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	*pType = pDevice->xInfo.xType;

	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_getDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff,
	FTDM_ULONG				nBuffLen
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL) && (pURL != NULL));

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	strncpy(pBuff, pDevice->xInfo.pURL, nBuffLen);

	return	FTDM_RET_OK;
}


FTDM_RET 	FTDM_setDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pURL
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL) && (pURL != NULL));

	if (strlen(pURL) > FTDM_DEVICE_URL_LEN)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	strcpy(pDevice->xInfo.pURL, pURL);

	FTDM_DBIF_setDeviceURL(pDID, pURL);

	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_getDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff, 
	FTDM_ULONG				nBuffLen
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL)  && (pBuff != NULL));

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	strncpy(pBuff, pDevice->xInfo.pLocation, nBuffLen);

	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_setDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pLocation 
)
{
	FTDM_RET		nRet;
	FTDM_DEVICE_PTR	pDevice;

	ASSERT((pDID != NULL) && (pLocation != NULL));

	if (strlen(pLocation) > FTDM_DEVICE_LOCATION_LEN)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getDevice(pDID, &pDevice);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	strcpy(pDevice->xInfo.pLocation, pLocation);

	FTDM_DBIF_setDeviceLocation(pDID, pLocation);

	return	FTDM_RET_OK;
}

