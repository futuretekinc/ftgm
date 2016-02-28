#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"
#include "ftdm_client_config.h"

typedef struct 
{
	FTM_INT	hSock;
	FTM_INT	nTimeout;
	sem_t	xLock;
}	FTDMC_SESSION, _PTR_ FTDMC_SESSION_PTR;

FTM_RET	FTDMC_init
(
	FTDMC_CFG_PTR pConfig
);

FTM_RET FTDMC_final
(
);

FTM_RET FTDMC_connect
(
	FTDMC_SESSION_PTR 		pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT 				usPort 
);

FTM_RET FTDMC_disconnect
(
	FTDMC_SESSION_PTR		pSession
);

FTM_RET FTDMC_isConnected
(
	FTDMC_SESSION_PTR		pSession,
	FTM_BOOL_PTR			pbConnected
);

FTM_RET FTDMC_NODE_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTDMC_NODE_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTDMC_NODE_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_NODE_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTDMC_NODE_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTDMC_EP_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_PTR			pInfo
);

FTM_RET FTDMC_EP_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID
);

FTM_RET FTDMC_EP_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTDMC_EP_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET	FTDMC_EP_CLASS_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDMC_EP_CLASS_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

FTM_RET	FTDMC_EP_CLASS_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_INFO_PTR	pInfo
);

FTM_RET FTDMC_EP_DATA_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pEPData
);

FTM_RET FTDMC_EP_DATA_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
);

FTM_RET FTDMC_EP_DATA_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nStartIndex,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_DATA_info
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pulCount
);

FTM_RET FTDMC_EP_DATA_getWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				ulBeginTime,
	FTM_ULONG				ulEndTime,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_DATA_removeWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				ulBeginTime,
	FTM_ULONG				ulEndTime
);

FTM_RET	FTDMC_EP_DATA_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_EP_DATA_countWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				ulBeginTime,
	FTM_ULONG				ulEndTime,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_EVENT_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EVENT_PTR     		pEvent
);

FTM_RET	FTDMC_EVENT_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EVENT_ID     		xEventID
);

FTM_RET	FTDMC_EVENT_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_EVENT_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EVENT_ID     		xEventID,
	FTM_EVENT_PTR			pEvent
);

FTM_RET	FTDMC_EVENT_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_EVENT_PTR			pEvent
);

FTM_RET	FTDMC_ACT_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ACT_PTR     		pAct
);

FTM_RET	FTDMC_ACT_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ACT_ID     			xActID
);

FTM_RET	FTDMC_ACT_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_ACT_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ACT_ID     			xActID,
	FTM_ACT_PTR				pAct
);

FTM_RET	FTDMC_ACT_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ACT_PTR				pAct
);

#endif

