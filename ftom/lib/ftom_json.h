#ifndef	__FTOM_JSON_H__
#define	__FTOM_JSON_H__

#include "ftom.h"
#include <cJSON.h>

typedef	cJSON	FTOM_JSON, _PTR_ FTOM_JSON_PTR;

FTM_RET	FTOM_JSON_createObject
(
	FTOM_JSON_PTR _PTR_ ppObject
);

FTM_RET	FTOM_JSON_createArray
(
	FTOM_JSON_PTR _PTR_ ppObject
);

FTM_RET	FTOM_JSON_destroy
(
	FTOM_JSON_PTR _PTR_ ppObject
);

FTM_RET	FTOM_JSON_addStringToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTM_CHAR_PTR	pString
);

FTM_RET	FTOM_JSON_addNumberToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTM_FLOAT		fValue
);

FTM_RET	FTOM_JSON_addItemToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTOM_JSON_PTR	pItem
);

FTM_RET	FTOM_JSON_addItemToArray
(
	FTOM_JSON_PTR	pObject,
	FTOM_JSON_PTR	pItem
);

FTM_RET	FTOM_JSON_getBufferSize
(
	FTOM_JSON_PTR	pObject,
	FTM_ULONG_PTR	pulSize
);

FTM_CHAR_PTR	FTOM_JSON_print
(
	FTOM_JSON_PTR	pRoot
);

FTM_RET	FTOM_JSON_createEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_JSON_PTR _PTR_	ppRoot
);

#endif
