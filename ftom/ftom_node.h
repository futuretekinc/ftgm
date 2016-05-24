#ifndef	__FTOM_NODE_H__
#define	__FTOM_NODE_H__

#include <pthread.h>
#include "ftm.h"
#include "ftom_ep.h"

typedef	enum
{
	FTOM_NODE_TYPE_SNMPC,
	FTOM_NODE_TYPE_MBC,
	FTOM_NODE_TYPE_FINSC,
	FTOM_NODE_TYPE_LORAM
} FTOM_NODE_TYPE, _PTR_ FTOM_NODE_TYPE_PTR;

typedef	FTM_ULONG		FTOM_NODE_STATE;


#define	FTOM_NODE_STATE_CREATED					0x00000001
#define	FTOM_NODE_STATE_INITIALIZED				0x00000002
#define	FTOM_NODE_STATE_RUN						0x00000003
#define	FTOM_NODE_STATE_STOP					0x00000004
#define	FTOM_NODE_STATE_FINALIZED				0x00000005


struct FTOM_NODE_STRUCT;
struct FTOM_EP_STRUCT;
struct FTM_EP_DATA_STRUCT;

typedef	FTM_RET		(*FTOM_NODE_INIT)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_FINAL)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_PRESTART)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_START)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_POSTSTART)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_PRESTOP)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_STOP)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_RET		(*FTOM_NODE_POSTSTOP)(struct FTOM_NODE_STRUCT _PTR_ pNode);
typedef	FTM_VOID_PTR (*FTOM_NODE_PROCESS)(FTM_VOID_PTR pNode);
typedef	FTM_RET		(*FTOM_NODE_GET_EP_DATA)(struct FTOM_NODE_STRUCT _PTR_ pNode, struct FTOM_EP_STRUCT _PTR_ pEP, struct FTM_EP_DATA_STRUCT _PTR_ pData);
typedef	FTM_RET		(*FTOM_NODE_SET_EP_DATA)(struct FTOM_NODE_STRUCT _PTR_ pNode, struct FTOM_EP_STRUCT _PTR_ pEP, struct FTM_EP_DATA_STRUCT _PTR_ pData);

typedef struct FTOM_NODE_DESCRIPT_STRUCT
{
	FTOM_NODE_TYPE	xType;

	FTOM_NODE_INIT			fInit;
	FTOM_NODE_FINAL			fFinal;
	FTOM_NODE_PRESTART		fPrestart;
	FTOM_NODE_START			fStart;
	FTOM_NODE_POSTSTART		fPoststart;
	FTOM_NODE_PRESTOP		fPrestop;
	FTOM_NODE_STOP			fStop;
	FTOM_NODE_POSTSTOP		fPoststop;
	FTOM_NODE_PROCESS		fProcess;
	FTOM_NODE_GET_EP_DATA	fGetEPData;
	FTOM_NODE_SET_EP_DATA	fSetEPData;
} FTOM_NODE_DESCRIPT, _PTR_ FTOM_NODE_DESCRIPT_PTR;

typedef	struct FTOM_NODE_STRUCT
{
	FTM_NODE				xInfo;
	FTM_LIST				xEPList;

	FTM_BOOL				bStop;
	pthread_t				xThread;
	pthread_mutex_t			xMutexLock;
	FTOM_NODE_STATE			xState;
	FTM_ULONG				ulRetry;
	int64_t					xTimeout;
	FTOM_MSG_QUEUE			xMsgQ;

	FTM_CHAR				pIP[32];
	FTOM_NODE_DESCRIPT		xDescript;
}	FTOM_NODE, _PTR_ FTOM_NODE_PTR;

FTM_RET	FTOM_NODE_init
(
	FTM_VOID
);

FTM_RET FTOM_NODE_final
(
	FTM_VOID
);

FTM_RET	FTOM_NODE_create
(
	FTM_NODE_PTR 		pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_createFromDB
(
	FTM_CHAR_PTR	pDID,
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_destroy
(
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_count
(
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTOM_NODE_get
(
	FTM_CHAR_PTR pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTOM_NODE_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTOM_NODE_set
(
	FTM_CHAR_PTR 	pDID, 
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
);

FTM_RET	FTOM_NODE_linkEP
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_NODE_unlinkEP
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_NODE_getEPCount
(
	FTOM_NODE_PTR	pNode, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_NODE_getEP
(
	FTOM_NODE_PTR 	pNode, 
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ pEP
);

FTM_RET	FTOM_NODE_getEPAt
(
	FTOM_NODE_PTR 	pNode, 
	FTM_ULONG 		ulIndex, 
	FTOM_EP_PTR _PTR_ pEP
);

FTM_RET	FTOM_NODE_getEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_NODE_setEPData
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);


FTM_RET	FTOM_NODE_start
(
	FTOM_NODE_PTR 	pNode
);

FTM_RET	FTOM_NODE_stop
(
	FTOM_NODE_PTR 	pNode
);

FTM_CHAR_PTR	FTOM_NODE_stateToStr
(
	FTOM_NODE_STATE xState
);

FTM_RET	FTOM_NODE_printList
(
	FTM_VOID
);

#include "ftom_node_snmpc.h"
#include "ftom_node_modbus_client.h"
#include "ftom_node_fins_client.h"

#endif

