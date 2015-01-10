#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_config.h"

FTM_RET	FTDM_init
(
	FTDM_CFG_PTR pConfig
);

FTM_RET	FTDM_final
(
	void
);

FTM_RET	FTDM_addNodeInfo
(
	FTM_NODE_INFO_PTR	pInfo
);

FTM_RET 	FTDM_delNodeInfo
(
	FTM_CHAR_PTR		pDID
);

FTM_RET	FTDM_getNodeInfoCount
(
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_getNodeInfo
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_getNodeInfoByIndex
(
	FTM_ULONG			nIndex,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_getNodeType
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_TYPE_PTR 	pType
);

FTM_RET	FTDM_addEPInfo
(
	FTM_EP_INFO_PTR 	pInfo
);

FTM_RET	FTDM_delEPInfo
(
	FTM_EPID 			xEPID
);

FTM_RET	FTDM_getEPInfoCount
(
	FTM_EP_CLASS		xClass,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_getEPInfo
(
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
);

FTM_RET	FTDM_getEPInfoByIndex
(
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
);

FTM_RET	FTDM_addEPClassInfo
(
	FTM_EP_CLASS_INFO_PTR	pInfo
);

FTM_RET	FTDM_delEPClassInfo
(
	FTM_EP_CLASS			xClass
);

FTM_RET	FTDM_getEPClassInfo
(
	FTM_EP_CLASS			xClass,
	FTM_EP_CLASS_INFO_PTR	pInfo
);

FTM_RET	FTDM_addEPData
(
	FTM_EPID			xEPID,
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTDM_getEPData
(
	FTM_EPID			xEPID, 
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_getEPDataWithTime
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_delEPData
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nIndex, 
	FTM_ULONG			nCount
); 

FTM_RET	FTDM_delEPDataWithTime
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime 
); 

FTM_RET	FTDM_getEPDataCount
(
	FTM_EPID			xEPID, 
	FTM_ULONG_PTR		pCount
); 

FTM_RET	FTDM_getEPDataCountWithTime
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pCount
);

#endif

