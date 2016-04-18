#ifndef	__FTOM_EP_MANAGEMENT_H__
#define	__FTOM_EP_MANAGEMENT_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftom.h"

typedef	struct FTOM_STRUCT _PTR_	FTOM_PTR;
typedef	struct FTOM_NODE_STRUCT _PTR_ FTOM_NODE_PTR;
typedef	struct FTOM_EP_STRUCT _PTR_ FTOM_EP_PTR;

typedef	struct FTOM_EPM_STRUCT
{
	FTOM_PTR		pOM;
	FTM_LIST_PTR	pEPList;
	FTM_LIST_PTR	pClassList;
}	FTOM_EPM, _PTR_ FTOM_EPM_PTR;

FTM_RET	FTOM_EPM_create
(
	FTOM_PTR 	pOM, 
	FTOM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTOM_EPM_destroy
(
	FTOM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTOM_EPM_init
(
	FTOM_PTR 		pOM, 
	FTOM_EPM_PTR 	pEPM
);

FTM_RET	FTOM_EPM_final
(
	FTOM_EPM_PTR 	pEPM
);

FTM_RET	FTOM_EPM_createEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_PTR 		pInfo, 
	FTOM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTOM_EPM_destroyEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_EPM_attachEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_EPM_detachEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_EPM_count
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_TYPE 	xType, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTOM_EPM_getIDList
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_TYPE 	xType, 
	FTM_CHAR		pEPID[][FTM_EPID_LEN+1],
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTOM_EPM_getEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ ppEP
);

FTM_RET FTOM_EPM_getEPAt
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_ULONG 		ulIndex, 
	FTOM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTOM_EPM_sendEPData
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		ulCount
);

FTM_RET	FTOM_EPM_createClass
(
	FTOM_EPM_PTR		pEPM,
	FTM_EP_CLASS_PTR  	pEPClass
);

FTM_RET	FTOM_EPM_destroyClass
(
	FTOM_EPM_PTR		pEPM,
	FTM_EP_CLASS_PTR  	pEPClass
);

FTM_RET	FTOM_EPM_countClass
(
	FTOM_EPM_PTR 		pEPM, 
	FTM_ULONG_PTR 		pulCount
);

FTM_RET FTOM_EPM_getClass
(
	FTOM_EPM_PTR 		pEPM, 
	FTM_EP_TYPE 		xType, 
	FTM_EP_CLASS_PTR _PTR_ ppEPClass
);

FTM_RET FTOM_EPM_getClassAt
(
	FTOM_EPM_PTR 		pEPM, 
	FTM_ULONG 			ulIndex, 
	FTM_EP_CLASS_PTR _PTR_ ppEPClass
);

FTM_RET	FTOM_EPM_saveEPData
(
	FTOM_EPM_PTR	pEPM,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
);

FTM_RET	FTOM_EPM_sendEPData
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		ulCount
);

#endif

