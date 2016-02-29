#ifndef	__SQLITE_IF_H__
#define	__SQLITE_IF_H__

#include "ftm_types.h"
#include "ftm_event.h"

FTM_RET	FTDM_DBIF_init
(
	FTM_VOID
);

FTM_RET	FTDM_DBIF_final
(
	void
);

FTM_RET	FTDM_DBIF_loadConfig
(
	FTDM_CFG_DB_PTR	pConfig
);

FTM_RET	FTDM_DBIF_NODE_initTable
(
	void
);

FTM_RET	FTDM_DBIF_NODE_count
(
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_NODE_getList
(
	FTM_NODE_PTR	pInfos,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_NODE_append
(
 	FTM_NODE_PTR	pInfo
);

FTM_RET	FTDM_DBIF_NODE_del
(
	FTM_CHAR_PTR		pDID
);

FTM_RET	FTDM_DBIF_NODE_get
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTDM_DBIF_NODE_setURL
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pURL
);

FTM_RET	FTDM_DBIF_NODE_getURL
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				nBuffLen
);

FTM_RET	FTDM_DBIF_NODE_setLocation
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pLocation
);

FTM_RET	FTDM_DBIF_NODE_getLocation
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				nBuffLen
);

FTM_RET	FTDM_DBIF_EP_initTable
(
	void
);

FTM_RET	FTDM_DBIF_EP_append
(
 	FTM_EP_PTR		pInfo
);

FTM_RET	FTDM_DBIF_EP_del
(
	FTM_EP_ID			xEPID
);

FTM_RET	FTDM_DBIF_EP_count
(
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_EP_getList
(
	FTM_EP_PTR		pInfos,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_EP_get
(
	FTM_EP_ID 			xEPID, 
	FTM_EP_PTR		pInfo
);

FTM_RET	FTDM_DBIF_EP_setName
(
	FTM_EP_ID			xEPID,
	FTM_CHAR_PTR		pName,
	FTM_INT				nNameLen
);

FTM_RET	FTDM_DBIF_EP_getName
(
	FTM_EP_ID			xEPID,
	FTM_CHAR_PTR		pName,
	FTM_INT_PTR			pNameLen
);

FTM_RET	FTDM_DBIF_EP_setInterval
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			nInterval
);

FTM_RET	FTDM_DBIF_EP_getInterval
(
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pInterval
);

FTM_RET	FTDM_DBIF_EP_setUnit
(
	FTM_EP_ID			xEPID,
	FTM_CHAR_PTR		pUnit,
	FTM_INT				nUnitLen
);

FTM_RET	FTDM_DBIF_EP_getUnit
(
	FTM_EP_ID			xEPID,
	FTM_CHAR_PTR		pUnit,
	FTM_INT_PTR			pUnitLen
);


FTM_RET	FTDM_DBIF_EP_DATA_initTable
(
	FTM_EP_ID			xEPID
);

FTM_RET	FTDM_DBIF_EP_DATA_append
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTDM_DBIF_EP_DATA_info
(
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pulBeginTime,
	FTM_ULONG_PTR		pulEndTime
);

FTM_RET	FTDM_DBIF_EP_DATA_get
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_EP_DATA_getWithTime
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_EP_DATA_del
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
);

FTM_RET	FTDM_DBIF_EP_DATA_delWithTime
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime
);

FTM_RET	FTDM_DBIF_EP_DATA_count
(
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_EP_DATA_countWithTime
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_ULONG_PTR		pCount
);

FTM_RET FTDM_DBIF_setTrace
(
	FTM_BOOL			bTraceON
);

FTM_RET FTDM_DBIF_getTrace
(
	FTM_BOOL_PTR		pbTranceON
);

FTM_RET	FTDM_DBIF_EVENT_initTable
(
	FTM_VOID
);

FTM_RET	FTDM_DBIF_EVENT_append
(
 	FTM_EVENT_PTR	pEvent
);

FTM_RET	FTDM_DBIF_EVENT_get
(
	FTM_EVENT_ID	xID,
 	FTM_EVENT_PTR	pEvent
);

FTM_RET	FTDM_DBIF_EVENT_getList
(
	FTM_EVENT_PTR		pEvents, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_DBIF_ACT_initTable
(
	FTM_VOID
);

FTM_RET	FTDM_DBIF_ACT_append
(
 	FTM_ACT_PTR		pAct
);

FTM_RET	FTDM_DBIF_ACT_get
(
	FTM_ACT_ID		xID,
 	FTM_ACT_PTR		pAct
);

FTM_RET	FTDM_DBIF_ACT_getList
(
	FTM_ACT_PTR			pActs, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
);
#endif
