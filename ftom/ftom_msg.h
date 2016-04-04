#ifndef	_FTOM_MSG_H_
#define	_FTOM_MSG_H_

#include "ftm.h"

#define	FTOM_MSG_STRING_LENGTH	1024
#define	FTOM_MSG_REQ_ID_LENGTH	32

typedef	FTM_ULONG	FTOM_MSG_ID, _PTR_ FTOM_MSG_ID_PTR;

typedef	enum
{
	FTOM_MSG_TYPE_QUIT				=	0,
	FTOM_MSG_TYPE_EP_CTRL			=	1,
	FTOM_MSG_TYPE_RULE				=	2,
	FTOM_MSG_TYPE_SET_EP_DATA		=	3,
	FTOM_MSG_TYPE_SEND_EP_DATA		=	4,
	FTOM_MSG_TYPE_SAVE_EP_DATA		=   5,
	FTOM_MSG_TYPE_PUBLISH_EP_DATA	=	6,
	FTOM_MSG_TYPE_TIME_SYNC			=   7,
	FTOM_MSG_TYPE_CONNECTED			=   8,
	FTOM_MSG_TYPE_DISCONNECTED		=   9,
	FTOM_MSG_TYPE_ACTION			=	10,
	FTOM_MSG_TYPE_MAX
}	FTOM_MSG_TYPE, _PTR_ FTOM_MSG_TYPE_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
} FTOM_MSG, _PTR_ FTOM_MSG_PTR;

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_EP_ID		xEPID;
	FTM_EP_DATA		xData;
}	FTOM_MSG_SET_EP_DATA, _PTR_ FTOM_MSG_SET_EP_DATA_PTR;

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_EP_ID		xEPID;
	FTM_EP_DATA		xData;
}	FTOM_MSG_SAVE_EP_DATA, _PTR_ FTOM_MSG_SAVE_EP_DATA_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_EP_ID		xEPID;
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_SEND_EP_DATA, _PTR_ FTOM_MSG_SEND_EP_DATA_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_EP_ID		xEPID;
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_PUBLISH_EP_DATA, _PTR_ FTOM_MSG_PUBLISH_EP_DATA_PTR;

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_ULONG		ulTime;
}	FTOM_MSG_TIME_SYNC, _PTR_ FTOM_MSG_TIME_SYNC_PTR;

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_EP_ID		xEPID;
	FTM_EP_CTRL		xCtrl;
	FTM_ULONG		ulDuration;	
}	FTOM_MSG_EP_CTRL, _PTR_ FTOM_MSG_EP_CTRL_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ACTION_ID	xActionID;
	FTM_BOOL		bActivate;
}	FTOM_MSG_ACTION, _PTR_ FTOM_MSG_ACTION_PTR;

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTOM_MSG_ID		xMsgID;
	FTM_RULE_ID		xRuleID;
	FTM_RULE_STATE	xRuleState;
}	FTOM_MSG_RULE, _PTR_ FTOM_MSG_RULE_PTR;

typedef struct
{
	FTM_MSG_QUEUE	xQueue;
} FTOM_MSG_QUEUE, _PTR_ FTOM_MSG_QUEUE_PTR;

FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_createSetEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_SET_EP_DATA_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_createSaveEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_SAVE_EP_DATA_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_createPublishEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PUBLISH_EP_DATA_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_createTimeSync
(
	FTM_ULONG			ulTime,
	FTOM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
);

FTM_RET FTOM_MSG_createEPCtrl
(
	FTM_EP_ID			xEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_EP_CTRL_PTR _PTR_ ppMsg
);

FTM_RET FTOM_MSG_createRule
(
	FTM_RULE_ID			xRuleID,
	FTM_RULE_STATE		xRuleState,
	FTOM_MSG_RULE_PTR _PTR_ ppMsg
);

FTM_RET FTOM_MSG_createAction
(
	FTM_ACTION_ID		xActionID,
	FTM_BOOL			bActivate,
	FTOM_MSG_ACTION_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_createSendEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_SEND_EP_DATA_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_destroy
(
	FTOM_MSG_PTR _PTR_ ppMsg
);

FTM_RET FTOM_MSGQ_create
(
	FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ
);

FTM_RET FTOM_MSGQ_destroy
(
	FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ
);

FTM_RET FTOM_MSGQ_init
(
	FTOM_MSG_QUEUE_PTR pMsgQ
);

FTM_RET FTOM_MSGQ_final
(
	FTOM_MSG_QUEUE_PTR pMsgQ
);

FTM_RET	FTOM_MSGQ_push
(
	FTOM_MSG_QUEUE_PTR 	pMsgQ, 
	FTOM_MSG_PTR 		pMsg
);

FTM_RET	FTOM_MSGQ_pop
(
	FTOM_MSG_QUEUE_PTR	pMsgQ, 
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

FTM_RET	FTOM_MSGQ_timedPop
(
	FTOM_MSG_QUEUE_PTR 	pMsgQ, 
	FTM_ULONG 				ulTimeout, 
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

#endif
