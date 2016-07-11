#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "libconfig.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_server.h"
#include "ftm_shared_memory.h"
#include "ftom_server_cmd.h"
#include "ftom_discovery.h"
#include "ftom_session.h"
#include "ftom_logger.h"

#ifndef	FTOM_TRACE_IO
//#define	FTOM_TRACE_IO		1
#endif

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTOM_SERVER_CALLBACK)FUN }

static 
FTM_VOID_PTR FTOM_SERVER_process
(
	FTM_VOID_PTR 	pData
);

static 
FTM_VOID_PTR FTOM_SERVER_serviceHandler
(
	FTM_VOID_PTR 	pData
);

static 
FTM_VOID_PTR FTOM_SERVER_publishProcess
(
	FTM_VOID_PTR 	pData
);

static
FTM_RET	FTOM_SERVER_addSubscribeSession
(
	FTOM_SERVER_PTR				pServer,
	FTM_INT						hSocket,
	struct  sockaddr_in	_PTR_ 	pPeer,
	FTOM_SESSION_PTR _PTR_		ppSession
);

static
FTM_RET	FTOM_SERVER_removeSubscribeSession
(
	FTOM_SERVER_PTR			pServer,
	FTOM_SESSION_PTR _PTR_ 	ppSession
);

static
FTM_VOID_PTR	FTOM_SERVER_processPipe
(
	FTM_VOID_PTR	pData
);

static
FTM_VOID_PTR	FTOM_SERVER_processSM
(
	FTM_VOID_PTR	pData
);

static 
FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SERVER_PTR			pServer,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_getAt
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_set
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_SET_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_run
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_RUN_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_RUN_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_stop
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_STOP_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_STOP_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_registerAtServer
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_REGISTER_AT_SERVER_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_REGISTER_AT_SERVER_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_setRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_SET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_getRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_GET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_NODE_setReportInterval
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_SET_REPORT_INTERVAL_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_REPORT_INTERVAL_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_CREATE_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_destroy
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_PARAMS_PTR			pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_PARAMS_PTR			pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_SET_PARAMS_PTR			pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_SET_PARAMS_PTR			pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_remoteSet
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REMOTE_SET_PARAMS_PTR			pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_REMOTE_SET_PARAMS_PTR			pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_registerAtServer
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_REGISTER_AT_SERVER_PARAMS_PTR pReq,
	FTM_ULONG ulReqLen,
	FTOM_RESP_EP_REGISTER_AT_SERVER_PARAMS_PTR pResp,
	FTM_ULONG ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_setRegistered
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_SET_SERVER_REGISTERED_PARAMS_PTR pReq,
	FTM_ULONG	ulReqLen,
	FTOM_RESP_EP_SET_SERVER_REGISTERED_PARAMS_PTR pResp,
	FTM_ULONG ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_getRegistered
(
	FTOM_SERVER_PTR pServer,
	FTOM_REQ_EP_GET_SERVER_REGISTERED_PARAMS_PTR pReq,
	FTM_ULONG ulReqLen,
	FTOM_RESP_EP_GET_SERVER_REGISTERED_PARAMS_PTR pResp,
	FTM_ULONG ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_setReportInterval
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_EP_SET_REPORT_INTERVAL_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_SET_REPORT_INTERVAL_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_del
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_DEL_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_DEL_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_delWithTime
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_type
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_TYPE_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_TYPE_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_getLast
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_LOG_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_LOG_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_LOG_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static
FTM_RET	FTOM_SERVER_DISCOVERY_start
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_START_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_START_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static
FTM_RET	FTOM_SERVER_DISCOVERY_getInfo
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_INFO_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_INFO_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static
FTM_RET	FTOM_SERVER_DISCOVERY_getNodeList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_NODE_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_NODE_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static
FTM_RET	FTOM_SERVER_DISCOVERY_getEPList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_EP_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);


static FTOM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTOM_CMD_NODE_CREATE,			FTOM_SERVER_NODE_create),
	MK_CMD_SET(FTOM_CMD_NODE_DESTROY,			FTOM_SERVER_NODE_destroy),
	MK_CMD_SET(FTOM_CMD_NODE_COUNT,				FTOM_SERVER_NODE_count),
	MK_CMD_SET(FTOM_CMD_NODE_GET,				FTOM_SERVER_NODE_get),
	MK_CMD_SET(FTOM_CMD_NODE_GET_AT,			FTOM_SERVER_NODE_getAt),
	MK_CMD_SET(FTOM_CMD_NODE_SET,				FTOM_SERVER_NODE_set),
	MK_CMD_SET(FTOM_CMD_NODE_REGISTER_AT_SERVER,FTOM_SERVER_NODE_registerAtServer),
	MK_CMD_SET(FTOM_CMD_NODE_SET_REGISTERED, 	FTOM_SERVER_NODE_setRegistered),
	MK_CMD_SET(FTOM_CMD_NODE_GET_REGISTERED, 	FTOM_SERVER_NODE_getRegistered),
	MK_CMD_SET(FTOM_CMD_NODE_SET_REPORT_INTERVAL, FTOM_SERVER_NODE_setReportInterval),
	MK_CMD_SET(FTOM_CMD_NODE_RUN,				FTOM_SERVER_NODE_run),
	MK_CMD_SET(FTOM_CMD_NODE_STOP,				FTOM_SERVER_NODE_stop),

	MK_CMD_SET(FTOM_CMD_EP_CREATE,				FTOM_SERVER_EP_create),
	MK_CMD_SET(FTOM_CMD_EP_DESTROY,				FTOM_SERVER_EP_destroy),
	MK_CMD_SET(FTOM_CMD_EP_COUNT,				FTOM_SERVER_EP_count),
	MK_CMD_SET(FTOM_CMD_EP_GET_LIST,			FTOM_SERVER_EP_getList),
	MK_CMD_SET(FTOM_CMD_EP_GET,					FTOM_SERVER_EP_get),
	MK_CMD_SET(FTOM_CMD_EP_GET_AT,				FTOM_SERVER_EP_getAt),
	MK_CMD_SET(FTOM_CMD_EP_SET,					FTOM_SERVER_EP_set),
	MK_CMD_SET(FTOM_CMD_EP_REMOTE_SET,			FTOM_SERVER_EP_remoteSet),
	MK_CMD_SET(FTOM_CMD_EP_REGISTER_AT_SERVER,	FTOM_SERVER_EP_registerAtServer),
	MK_CMD_SET(FTOM_CMD_EP_SET_REGISTERED, 		FTOM_SERVER_EP_setRegistered),
	MK_CMD_SET(FTOM_CMD_EP_GET_REGISTERED, 		FTOM_SERVER_EP_getRegistered),
	MK_CMD_SET(FTOM_CMD_EP_SET_REPORT_INTERVAL, FTOM_SERVER_EP_setReportInterval),

	MK_CMD_SET(FTOM_CMD_EP_REG_NOTIFY_RECEIVER, FTOM_SERVER_EP_registrationNotifyReceiver),
	MK_CMD_SET(FTOM_CMD_EP_DATA_DEL,			FTOM_SERVER_EP_DATA_del),
	MK_CMD_SET(FTOM_CMD_EP_DATA_DEL_WITH_TIME,	FTOM_SERVER_EP_DATA_delWithTime),
	MK_CMD_SET(FTOM_CMD_EP_DATA_INFO,			FTOM_SERVER_EP_DATA_info),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LAST,		FTOM_SERVER_EP_DATA_getLast),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LIST,		FTOM_SERVER_EP_DATA_getList),
	MK_CMD_SET(FTOM_CMD_EP_DATA_COUNT,			FTOM_SERVER_EP_DATA_count),
	MK_CMD_SET(FTOM_CMD_EP_DATA_TYPE,			FTOM_SERVER_EP_DATA_type),

	MK_CMD_SET(FTOM_CMD_TRIG_ADD,				FTOM_SERVER_TRIGGER_add),
	MK_CMD_SET(FTOM_CMD_TRIG_DEL,				FTOM_SERVER_TRIGGER_del),
	MK_CMD_SET(FTOM_CMD_TRIG_COUNT,				FTOM_SERVER_TRIGGER_count),
	MK_CMD_SET(FTOM_CMD_TRIG_GET,				FTOM_SERVER_TRIGGER_get),
	MK_CMD_SET(FTOM_CMD_TRIG_GET_AT,			FTOM_SERVER_TRIGGER_getAt),
	MK_CMD_SET(FTOM_CMD_TRIG_SET,				FTOM_SERVER_TRIGGER_set),
	
	MK_CMD_SET(FTOM_CMD_ACTION_ADD,				FTOM_SERVER_ACTION_add),
	MK_CMD_SET(FTOM_CMD_ACTION_DEL,				FTOM_SERVER_ACTION_del),
	MK_CMD_SET(FTOM_CMD_ACTION_COUNT,			FTOM_SERVER_ACTION_count),
	MK_CMD_SET(FTOM_CMD_ACTION_GET,				FTOM_SERVER_ACTION_get),
	MK_CMD_SET(FTOM_CMD_ACTION_GET_AT,			FTOM_SERVER_ACTION_getAt),
	MK_CMD_SET(FTOM_CMD_ACTION_SET,				FTOM_SERVER_ACTION_set),
	
	MK_CMD_SET(FTOM_CMD_RULE_ADD,				FTOM_SERVER_RULE_add),
	MK_CMD_SET(FTOM_CMD_RULE_DEL,				FTOM_SERVER_RULE_del),
	MK_CMD_SET(FTOM_CMD_RULE_COUNT,				FTOM_SERVER_RULE_count),
	MK_CMD_SET(FTOM_CMD_RULE_GET,				FTOM_SERVER_RULE_get),
	MK_CMD_SET(FTOM_CMD_RULE_GET_AT,			FTOM_SERVER_RULE_getAt),
	MK_CMD_SET(FTOM_CMD_RULE_SET,				FTOM_SERVER_RULE_set),
	
	MK_CMD_SET(FTOM_CMD_LOG_DEL,				FTOM_SERVER_LOG_del),
	MK_CMD_SET(FTOM_CMD_LOG_COUNT,				FTOM_SERVER_LOG_count),
	MK_CMD_SET(FTOM_CMD_LOG_GET_LIST,			FTOM_SERVER_LOG_getList),

	MK_CMD_SET(FTOM_CMD_DISCOVERY_START,		FTOM_SERVER_DISCOVERY_start),
	MK_CMD_SET(FTOM_CMD_DISCOVERY_GET_INFO,		FTOM_SERVER_DISCOVERY_getInfo),
	MK_CMD_SET(FTOM_CMD_DISCOVERY_GET_NODE,		FTOM_SERVER_DISCOVERY_getNodeList),
	MK_CMD_SET(FTOM_CMD_DISCOVERY_GET_EP,		FTOM_SERVER_DISCOVERY_getEPList),

	MK_CMD_SET(FTOM_CMD_UNKNOWN, 		NULL)
};

FTM_RET	FTOM_SERVER_create
(
	FTOM_SERVER_PTR _PTR_ 	ppServer
)
{
	ASSERT(ppServer != NULL);

	FTM_RET	xRet;
	FTOM_SERVER_PTR	pServer;

	pServer = (FTOM_SERVER_PTR)FTM_MEM_malloc(sizeof(FTOM_SERVER));
	if (pServer == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_SERVER_init(pServer);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pServer);
		return	xRet;	
	}

	*ppServer = pServer;

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_SERVER_destroy
(
	FTOM_SERVER_PTR _PTR_ 	ppServer
)
{
	ASSERT(ppServer != NULL);

	FTOM_SERVER_final(*ppServer);

	FTM_MEM_free(*ppServer);

	*ppServer = NULL;

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_SERVER_init
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	memset(pServer, 0, sizeof(FTOM_SERVER));

	memset(&pServer->xConfig, 0, sizeof(FTOM_SERVER_CONFIG));

	pServer->xConfig.usPort			= FTOM_DEFAULT_SERVER_PORT;
	pServer->xConfig.ulMaxSession	= FTOM_DEFAULT_SERVER_SESSION_COUNT	;
	FTM_LIST_init(&pServer->xSessionList);

	pServer->xConfig.xPublisher.usPort			= 9000;
	pServer->xConfig.xPublisher.ulMaxSubscribe	= FTOM_DEFAULT_SERVER_SESSION_COUNT	;
	FTM_LIST_init(&pServer->xPublisher.xSubscriberList);

	if (sem_init(&pServer->xPublisher.xLock, 0, 1) < 0)
	{
		ERROR2(FTM_RET_FAILED_TO_INIT_SEM, "Failed to initialize semaphore!\n");
		FTM_LIST_final(&pServer->xPublisher.xSubscriberList);
		FTM_LIST_final(&pServer->xSessionList);
		return	FTM_RET_FAILED_TO_INIT_SEM;
	}

	if (sem_init(&pServer->xPublisher.xSlot, 0, pServer->xConfig.xPublisher.ulMaxSubscribe) < 0)
	{
		ERROR2(FTM_RET_FAILED_TO_INIT_SEM, "Failed to initialize semaphore!\n");
		sem_destroy(&pServer->xPublisher.xLock);
		FTM_LIST_final(&pServer->xPublisher.xSubscriberList);
		FTM_LIST_final(&pServer->xSessionList);
		return	FTM_RET_FAILED_TO_INIT_SEM;
	}


	pServer->bStop = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_final
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SERVER_stop(pServer);

	FTM_LIST_final(&pServer->xSessionList);
	FTM_LIST_final(&pServer->xPublisher.xSubscriberList);
	sem_destroy(&pServer->xPublisher.xSlot);
	sem_destroy(&pServer->xPublisher.xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_start
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_INT	nRet;

	if (!pServer->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	pServer->bStop = FTM_FALSE;
	
	nRet = pthread_create(&pServer->xPThread, NULL, FTOM_SERVER_process, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create Net interface[%d]\n", nRet);
	}

	nRet = pthread_create(&pServer->xPublisher.xThread, NULL, FTOM_SERVER_publishProcess, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create Net interface[%d]\n", nRet);
	}

	nRet = pthread_create(&pServer->xProcessPipe, NULL, FTOM_SERVER_processPipe, pServer);
	if (nRet != 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create PIPE interface[%d]\n", nRet);
	}

	nRet = pthread_create(&pServer->xProcessSM, NULL, FTOM_SERVER_processSM, pServer);
	if (nRet != 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create SM interface[%d]\n", nRet);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_stop
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SESSION_PTR	pSession = NULL;

	if (pServer->bStop)
	{
		return	FTM_RET_NOT_START;
	}

	FTM_LIST_iteratorStart(&pServer->xPublisher.xSubscriberList);
	while(FTM_LIST_iteratorNext(&pServer->xPublisher.xSubscriberList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, NULL);

		FTM_MEM_free(pSession);		
	}

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, NULL);

		FTM_MEM_free(pSession);		
	}

	pServer->bStop = FTM_TRUE;
	shutdown(pServer->hSocket, SHUT_RD);
	pthread_join(pServer->xPThread, NULL);
	pthread_join(pServer->xProcessPipe, NULL);
	pthread_join(pServer->xProcessSM, NULL);

	TRACE("Server finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_isRun
(
	FTOM_SERVER_PTR	pServer,
	FTM_BOOL_PTR	pbRun
)
{
	ASSERT(pServer != NULL);
	ASSERT(pbRun != NULL);

	if ((pServer != NULL) && (!pServer->bStop))
	{
		*pbRun = FTM_TRUE;	
	}
	else
	{
		*pbRun = FTM_FALSE;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_sendMessage
(
	FTOM_SERVER_PTR	pServer,
	FTOM_MSG_PTR 		pMsg
)
{
	ASSERT(pServer != NULL);
	ASSERT(pMsg != NULL);
	
	FTOM_SESSION_PTR	pSession = NULL;

	FTM_LIST_iteratorStart(&pServer->xPublisher.xSubscriberList);
	while(FTM_LIST_iteratorNext(&pServer->xPublisher.xSubscriberList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		FTOM_REQ_NOTIFY_PARAMS_PTR	pParam = (FTOM_REQ_NOTIFY_PARAMS_PTR)pSession->pReqBuff;
		FTM_ULONG	ulParamLen  = sizeof(FTOM_REQ_NOTIFY_PARAMS) - sizeof(FTOM_MSG) + pMsg->ulLen;
		FTM_INT		nSendLen = 0;
		FTM_INT		nRecvLen = 0;

		pParam->ulReqID = pSession->ulReqID++;
		pParam->xCmd	= FTOM_CMD_SERVER_NOTIFY;
		pParam->ulLen	= sizeof(FTOM_REQ_NOTIFY_PARAMS) - sizeof(FTOM_MSG) + pMsg->ulLen;
		memcpy(&pParam->xMsg, pMsg, pMsg->ulLen);

		nRecvLen = recv(pSession->hSocket, pSession->pRespBuff, sizeof(pSession->pRespBuff), MSG_DONTWAIT);
		if (nRecvLen == 0)
		{
			close(pSession->hSocket);
			FTOM_SERVER_removeSubscribeSession(pServer, &pSession);
		}
		else
		{
			nSendLen = send(pSession->hSocket, pParam, ulParamLen, 0);
			if(nSendLen == ulParamLen )
			{
				nRecvLen = recv(pSession->hSocket, pSession->pRespBuff, sizeof(pSession->pRespBuff), 0);
			}
	
			if (nSendLen <= 0)
			{
				close(pSession->hSocket);
				FTOM_SERVER_removeSubscribeSession(pServer, &pSession);
			}
		}	
	}

	FTOM_MSG_destroy(&pMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_setServiceCallback
(
	FTOM_SERVER_PTR	pServer, 
	FTOM_SERVICE_ID	xServiceID, 
	FTOM_SERVICE_CB	fServiceCB
)
{
	ASSERT(pServer != NULL);
	ASSERT(fServiceCB != NULL);

	pServer->xServiceID = xServiceID;
	pServer->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTM_INT				nRet;
	struct sockaddr_in	xServerAddr, xClientAddr;

	
	if (sem_init(&pServer->xLock, 0,pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_SEMAPHORE,"Can't alloc semaphore!\n");
		return	0;	
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->hSocket == -1)
	{
		ERROR2(FTM_RET_COMM_SOCK_ERROR, "Could not create socket\n");
		return	0;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pServer->xConfig.usPort );

	TRACE("Server[ %s:%d ]\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));
	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
	if (nRet < 0)
	{
		ERROR2(FTM_RET_COMM_SOCKET_BIND_FAILED, "bind failed.[%d]\n", nRet);
		return	0;
	}

	listen(pServer->hSocket, 3);

	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	
		struct timespec			xTimeout ;

		clock_gettime(CLOCK_REALTIME, &xTimeout);

		xTimeout.tv_sec += 2;
		if (sem_timedwait(&pServer->xLock, &xTimeout) == 0)
		{
			sem_getvalue(&pServer->xLock, &nValue);
			TRACE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(pServer->hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient > 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

				FTOM_SESSION_PTR pSession = (FTOM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTOM_SESSION));
				if (pSession == NULL)
				{
					ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "System memory is not enough!\n");
					TRACE("The session[%08x] was closed.\n", hClient);
					close(hClient);
				}
				else
				{
					TRACE("The new session[%08x] has beed connected\n", hClient);

					pSession->hSocket = hClient;
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pSession->pData = (FTM_VOID_PTR)pServer;
					if (pthread_create(&pSession->xPThread, NULL, FTOM_SERVER_serviceHandler, pSession) == 0)
					{
						FTM_LIST_append(&pServer->xSessionList, pSession);	
					}
					else
					{
						FTM_MEM_free(pSession);
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_serviceHandler
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_SESSION_PTR		pSession= (FTOM_SESSION_PTR)pData;
	FTOM_REQ_PARAMS_PTR		pReq 	= (FTOM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTOM_RESP_PARAMS_PTR	pResp 	= (FTOM_RESP_PARAMS_PTR)pSession->pRespBuff;

	pSession->bStop = FTM_FALSE;
	while(!pSession->bStop)
	{
		FTM_INT	ulReqLen;
		FTM_INT	ulSendLen;

		ulReqLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		if (ulReqLen == 0)
		{
			TRACE("The connection is terminated.\n");
			pSession->bStop = FTM_TRUE;
		}
		else if (ulReqLen == -1)
		{
			WARN("Error packet received.\n");	
		}
		else if (ulReqLen > 0)
		{
			FTM_ULONG	ulRetry = 3;
#if	FTOM_TRACE_IO
			TRACE("RECV[%08lx:%08x] : Len = %lu\n", pSession->hSocket, pReq->ulReqID, ulReqLen);
#endif
			pResp->ulReqID = pReq->ulReqID;

			if (FTM_RET_OK != FTOM_SERVER_serviceCall((FTOM_SERVER_PTR)pSession->pData, pReq, ulReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE))
			{
				pResp->xCmd = pReq->xCmd;
				pResp->xRet = FTM_RET_INTERNAL_ERROR;
				pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
			}

#if	FTOM_TRACE_IO
			TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->ulReqID, pResp->ulLen);
#endif
			do
			{
				ulSendLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
			}
			while ((--ulRetry > 0) && (ulSendLen < 0));

			if (ulSendLen < 0)
			{
				ERROR2(FTM_RET_ERROR, "send failed[%d]\n", -ulSendLen);	
				pSession->bStop = FTM_TRUE;
			}
		}
		else if (ulReqLen < 0)
		{
			ERROR2(FTM_RET_ERROR, "recv failed.[%d]\n", -ulReqLen);
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	FTM_LIST_remove(&((FTOM_SERVER_PTR)pSession->pData)->xSessionList, pSession);	
	sem_post(&((FTOM_SERVER_PTR)pSession->pData)->xLock);
	FTM_MEM_free(pSession);
	return	0;
}

FTM_VOID_PTR FTOM_SERVER_publishProcess
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	
	FTM_RET	xRet;
	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTM_INT				nRet;
	struct sockaddr_in	xLocalAddr;
	FTOM_SESSION_PTR	pSession;

	
	if (sem_init(&pServer->xPublisher.xLock, 0, 1) < 0)
	{
		ERROR2(FTM_RET_ERROR, "Can't alloc semaphore!\n");
		return	0;	
	}

	if (sem_init(&pServer->xPublisher.xSlot, 0, pServer->xConfig.xPublisher.ulMaxSubscribe) < 0)
	{
		ERROR2(FTM_RET_ERROR, "Can't alloc semaphore!\n");
		return	0;	
	}

	pServer->xPublisher.hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->xPublisher.hSocket == -1)
	{
		ERROR2(FTM_RET_ERROR, "Could not create socket\n");
		return	0;
	}

	xLocalAddr.sin_family 		= AF_INET;
	xLocalAddr.sin_addr.s_addr = INADDR_ANY;
	xLocalAddr.sin_port 		= htons( pServer->xConfig.xPublisher.usPort );

	TRACE("Local [ %s:%d ]\n", inet_ntoa(xLocalAddr.sin_addr), ntohs(xLocalAddr.sin_port));
	nRet = bind( pServer->xPublisher.hSocket, (struct sockaddr *)&xLocalAddr, sizeof(xLocalAddr));
	if (nRet < 0)
	{
		ERROR2(FTM_RET_COMM_SOCKET_BIND_FAILED, "bind failed.[%d]\n", nRet);
		return	0;
	}

	TRACE("Publisher started.\n");
	listen(pServer->xPublisher.hSocket, 3);

	while(!pServer->bStop)
	{
		FTM_INT	nValue;
		FTM_INT	hClient;
		struct  sockaddr_in	xRemoteAddr;
		FTM_INT	nRemoteAddrLen = sizeof(xRemoteAddr);	


		sem_getvalue(&pServer->xPublisher.xSlot, &nValue);
		TRACE("Waiting for subscriber[%d]...\n", nValue);
		hClient = accept(pServer->xPublisher.hSocket, (struct sockaddr *)&xRemoteAddr, (socklen_t *)&nRemoteAddrLen);
		if (hClient > 0)
		{

			TRACE("Accept new subscriber.[ %s:%d ]\n", inet_ntoa(xRemoteAddr.sin_addr), ntohs(xRemoteAddr.sin_port));

			xRet = FTOM_SERVER_addSubscribeSession(pServer, hClient, &xRemoteAddr, &pSession);
			if (xRet != FTM_RET_OK)
			{
				close(hClient);
			}
		}

	}
	TRACE("Publisher stopped.\n");

	FTM_LIST_iteratorStart(&pServer->xPublisher.xSubscriberList);
	while(FTM_LIST_iteratorNext(&pServer->xPublisher.xSubscriberList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		close(pSession->hSocket);
		FTOM_SERVER_removeSubscribeSession(pServer, &pSession);
	}

	sem_destroy(&pServer->xPublisher.xLock);
	sem_destroy(&pServer->xPublisher.xSlot);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_addSubscribeSession
(
	FTOM_SERVER_PTR				pServer,
	FTM_INT						hSocket,
	struct  sockaddr_in	_PTR_ 	pPeer,
	FTOM_SESSION_PTR _PTR_		ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppSession != NULL);

	FTM_RET				xRet;
	FTM_INT				nRet;
	struct  timespec	xTimeout ;
	FTOM_SESSION_PTR	pSession;

	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_sec += 1;
	nRet =sem_timedwait(&pServer->xPublisher.xSlot, &xTimeout);
	if (nRet != 0)
	{
		ERROR2(FTM_RET_COMM_SESSION_IS_FULL, "Session is full!\n");
		return	FTM_RET_COMM_SESSION_IS_FULL;
	}

	pSession = (FTOM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTOM_SESSION));
	if (pSession == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!n");
		sem_post(&pServer->xPublisher.xSlot);
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pSession->hSocket = hSocket;
	memcpy(&pSession->xPeer, pPeer, sizeof(struct sockaddr_in));
	pSession->pData = (FTM_VOID_PTR)pServer;

	sem_wait(&pServer->xPublisher.xLock);
	xRet = FTM_LIST_append(&pServer->xPublisher.xSubscriberList, pSession);	
	sem_post(&pServer->xPublisher.xLock);

	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pSession);	
		sem_post(&pServer->xPublisher.xSlot);
	}
	else
	{
		*ppSession = pSession;	
		TRACE("New subscribe session[%d]\n", (*ppSession)->hSocket);
	}

	return	xRet;
}

FTM_RET	FTOM_SERVER_removeSubscribeSession
(
	FTOM_SERVER_PTR			pServer,
	FTOM_SESSION_PTR _PTR_ 	ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppSession != NULL);
	FTM_RET	xRet;

	sem_wait(&pServer->xPublisher.xLock);
	xRet = FTM_LIST_remove(&pServer->xPublisher.xSubscriberList, (*ppSession));
	sem_post(&pServer->xPublisher.xLock);
	if (xRet == FTM_RET_OK)
	{
		TRACE("Remove subscribe session[%d]\n", (*ppSession)->hSocket);
		FTM_MEM_free(*ppSession);	
		sem_post(&pServer->xPublisher.xSlot);
	}

	return	xRet;
}

FTM_VOID_PTR	FTOM_SERVER_processPipe
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET				xRet;
	FTM_INT				nReadFD, nWriteFD;
	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	fd_set				xFDSet;
	struct timeval		xTimeout;
	FTOM_REQ_PARAMS_PTR		pReq;
	FTOM_RESP_PARAMS_PTR	pResp;

	pReq	= (FTOM_REQ_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pReq == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	0;	
	}

	pResp 	= (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pResp == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		FTM_MEM_free(pReq);
		return	0;	
	}

	mkfifo("/tmp/ftom_in", 0666);
	mkfifo("/tmp/ftom_out", 0666);

	nReadFD = open("/tmp/ftom_in", O_RDWR);
	nWriteFD= open("/tmp/ftom_out", O_RDWR);

	FD_ZERO(&xFDSet); 			/* clear the set */

	while(!pServer->bStop)
	{
		FTM_INT	nRet;
		FTM_INT	nReqLen;

    	xTimeout.tv_sec = 1;
		xTimeout.tv_usec = 00000;
		
  		FD_SET(nReadFD, &xFDSet); 	/* add our file descriptor to the set */
		nRet = select(nReadFD + 1, &xFDSet, NULL, NULL, &xTimeout);
		if (nRet > 0)
		{
			nReqLen = read(nReadFD, pReq, FTOM_DEFAULT_PACKET_SIZE);	
			if (nReqLen > 0)
			{

				xRet = FTOM_SERVER_serviceCall(pServer, pReq, nReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to call PIPE Service[%08x]\n", pReq->xCmd);
					pResp->xCmd = pReq->xCmd;
					pResp->xRet = xRet;
					pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
				}

				write(nWriteFD, pResp, pResp->ulLen);
				//usleep(1);
#if 0
				FTM_INT	i;

				MESSAGE("WRITE : ");
				for(i = 0 ; i < pResp->ulLen ; i++)
				{
					MESSAGE("%02x ", ((FTM_UINT8_PTR)pResp)[i]);	
				}
				MESSAGE("\n");
#endif
			}
		}
	}

	close(nReadFD);
	close(nWriteFD);

	FTM_MEM_free(pReq);
	FTM_MEM_free(pResp);

	return	0;
}

FTM_VOID_PTR	FTOM_SERVER_processSM
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET				xRet;
	FTM_SMP_PTR			pSMP;
	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTOM_REQ_PARAMS_PTR		pReq;
	FTOM_RESP_PARAMS_PTR	pResp;

	pReq	= (FTOM_REQ_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pReq == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	0;	
	}

	pResp 	= (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pResp == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		FTM_MEM_free(pReq);
		return	0;	
	}

	xRet = FTM_SMP_createServer(1234, &pSMP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SMP creation failed.\n");
		return	0;	
	}

	FTM_SMP_createKeyFile(pSMP, pServer->xConfig.xSM.pKeyFile);

	while(!pServer->bStop)
	{
		FTM_ULONG	ulReqLen = 0;

		memset(pReq, 0, FTOM_DEFAULT_PACKET_SIZE);
		memset(pResp, 0, FTOM_DEFAULT_PACKET_SIZE);

		xRet = FTM_SMP_receiveReq(pSMP, pReq, FTOM_DEFAULT_PACKET_SIZE, &ulReqLen, 1000000);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_SERVER_serviceCall(pServer, pReq, ulReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to call PIPE Service[%08x]\n", pReq->xCmd);
				pResp->xCmd = pReq->xCmd;
				pResp->xRet = xRet;
				pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
			}

			xRet = FTM_SMP_sendResp(pSMP, pResp, pResp->ulLen, 100000);
		}
	}

	FTM_SMP_destroy(&pSMP);

	FTM_MEM_free(pReq);
	FTM_MEM_free(pResp);

	return	0;
}

FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SERVER_PTR			pServer,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_SERVER_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTOM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
#if	FTOM_TRACE_IO
			FTM_RET	xRet;

			TRACE("CMD : %s\n", pSet->pCmdString);
			xRet = 
#endif
			pSet->fService(pServer, pReq, ulReqLen, pResp, ulRespLen);
#if	FTOM_TRACE_IO
			TRACE("RET : %08lx\n", xRet);
#endif
			return	FTM_RET_OK;
		}

		pSet++;
	}

	ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "FUNCTION NOT SUPPORTED\n");
	ERROR2(FTM_RET_INVALID_COMMAND,  "CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

/****************************************************************************************************
 *
 ****************************************************************************************************/
FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode = NULL;

	TRACE("Received a request to create a Node[%s].\n", pReq->xNodeInfo.pDID);
	xRet = FTOM_NODE_get(pReq->xNodeInfo.pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_RET_ALREADY_EXISTS;	
		goto finish;
	}

	xRet = FTOM_NODE_create(&pReq->xNodeInfo, FTM_TRUE, &pNode);
	if(xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node[%s]\n", pReq->xNodeInfo.pDID);
		goto finish;
	}

	xRet = FTOM_NODE_start(pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start node[%s]\n", pNode->xInfo.pDID);
	}

finish:
	if (xRet == FTM_RET_OK)
	{
		FTOM_LOG_createNode(&pNode->xInfo);	
		strcpy(pResp->pDID, pNode->xInfo.pDID);
	}
	else if (pNode != NULL)
	{
		FTOM_NODE_stop(pNode);
		FTOM_NODE_destroy(&pNode);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}


FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTM_NODE		xInfo;
	FTOM_EP_PTR		pEP;
	FTOM_NODE_PTR	pNode;
	FTM_ULONG		ulCount = 0;

	TRACE("Received request to destroy Node[%s].\n", pReq->pDID);
	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Faile to get node[%s]\n", pReq->pDID);
		goto finish;
	}

	xRet = FTOM_NODE_stop(pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop Node[%s]\n", pNode->xInfo.pDID);
		goto finish;
	}

	xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to ep count of node[%s].\n", pNode->xInfo.pDID);
		goto finish;	
	}

	TRACE("Node[%s] has %lu EP.\n", pNode->xInfo.pDID, ulCount);
	while(FTM_LIST_getFirst(&pNode->xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTM_EP	xEPInfo;

		memcpy(&xEPInfo, &pEP->xInfo, sizeof(FTM_EP));

		xRet = FTOM_NODE_unlinkEP(pNode, pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to unlink EP[%s].\n", pEP->xInfo.pEPID);
			goto finish;
		}

		xRet = FTOM_EP_destroy(&pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy EP[%s].\n", xEPInfo.pEPID);
			goto finish;
		}
	
		ulCount--;
		TRACE("Remain EP Count : %lu\n", ulCount);
		FTOM_LOG_destroyEP(xEPInfo.pEPID);
	}

	memcpy(&xInfo, &pNode->xInfo, sizeof(FTM_NODE));

	xRet = FTOM_NODE_destroy(&pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy Node[%s].\n", xInfo.pDID);
		goto finish;
	}

	FTOM_LOG_destroyNode(&xInfo);

finish:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_NODE_count(&pResp->ulCount);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_getAt
(	
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_NODE_getAt(pReq->ulIndex, &pNode);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	pResp->xRet = FTOM_NODE_set(pReq->pDID, pReq->xFields, &pReq->xInfo);

	if (pResp->xRet == FTM_RET_OK)
	{
		FTOM_NODE_PTR	pNode = NULL;
		pResp->xRet = FTOM_NODE_get(pReq->pDID, &pNode);
		if (pResp->xRet == FTM_RET_OK)
		{
			memcpy(&pResp->xInfo, &pNode->xInfo, sizeof(FTM_NODE));
		}
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_run
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_RUN_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_RUN_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_NODE_start(pNode);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_stop
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_STOP_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_STOP_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_NODE_stop(pNode);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_registerAtServer
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_REGISTER_AT_SERVER_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_REGISTER_AT_SERVER_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_NODE_setRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_SET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		pNode->xServer.bRegistered = pReq->bRegistered;
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_NODE_getRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_GET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		pResp->bRegistered = pNode->xServer.bRegistered;
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_NODE_setReportInterval
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_SET_REPORT_INTERVAL_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_REPORT_INTERVAL_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_NODE_get(pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_NODE_setReportInterval(pNode, pReq->ulReportInterval);
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTOM_NODE_PTR	pNode;

	xRet = FTOM_NODE_get(pReq->xInfo.pDID, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Node[%s] not found for EP[%s]!\n", pReq->xInfo.pDID, pReq->xInfo.pEPID);
		goto finish;
	}

	xRet = FTOM_EP_create(&pReq->xInfo, FTM_TRUE, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create EP[%s].\n", pReq->xInfo.pEPID);
		goto finish;
	}

	FTOM_NODE_linkEP(pNode, pEP);

	memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));

	xRet = FTOM_EP_start(pEP);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to start EP[%s]!\n", pEP->xInfo.pEPID);
	}

	FTOM_LOG_createEP(pEP->xInfo.pEPID);

finish:
	pResp->xCmd 	= pReq->xCmd;
	pResp->ulLen 	= sizeof(*pResp);
	pResp->xRet 	= xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_EP_destroy
(		
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	FTOM_EP_PTR		pEP;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to find EP[%s].\n", pReq->pEPID);
		goto finish;
	}

	xRet = FTOM_NODE_get(pEP->xInfo.pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_NODE_unlinkEP(pNode, pEP);	
		if (xRet != FTM_RET_OK)
		{
			goto finish;	
		}
	}

	xRet = FTOM_LOG_destroyEP(pEP->xInfo.pEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete log of EP[%s].\n", pEP->xInfo.pEPID);	
	}

	xRet = FTOM_EP_destroy(&pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove EP[%s].\n", pEP->xInfo.pEPID);
		goto finish;
	}

finish:

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EP_count(pReq->xType, pReq->pDID, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_getList
(	
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->xRet = FTOM_EP_getIDList(pReq->xType, pReq->pDID, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + (FTM_EPID_LEN + 1) * pResp->ulCount;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EP_getAt(pReq->ulIndex, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}
	
	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_SET_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_SET_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet= FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_setInfo(pEP, pReq->xFields, &pReq->xInfo);
	}


	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_EP_remoteSet
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REMOTE_SET_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_REMOTE_SET_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet= FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_remoteSetData(pEP, &pReq->xData);
	}


	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_EP_registerAtServer
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_EP_REGISTER_AT_SERVER_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_REGISTER_AT_SERVER_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_EP_setRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_EP_SET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_SET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET			xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		pEP->xServer.bRegistered = pReq->bRegistered;
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_EP_getRegistered
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_EP_GET_SERVER_REGISTERED_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_GET_SERVER_REGISTERED_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		pResp->bRegistered = pEP->xServer.bRegistered;
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_EP_setReportInterval
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_EP_SET_REPORT_INTERVAL_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_SET_REPORT_INTERVAL_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_setReportInterval(pEP, pReq->ulReportInterval);
	}
	pResp->xRet = xRet;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	FTM_RET_OK;
}



FTM_RET	FTOM_SERVER_EP_DATA_del
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_DEL_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_DEL_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_removeData( pEP, pReq->ulIndex, pReq->ulCount, &pResp->ulCount);	
	}
	else
	{
		ERROR2(xRet, "EP[%s] not found.\n", pReq->pEPID);	
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_delWithTime
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_removeDataWithTime( pEP, pReq->ulBegin, pReq->ulEnd, &pResp->ulCount);
	}
	else
	{
		ERROR2(xRet, "EP[%s] not found.\n", pReq->pEPID);	
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_getDataInfo( pEP, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);
	}
	else
	{
		ERROR2(xRet, "EP[%s] not found.\n", pReq->pEPID);	
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_getLast
(	
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer!= NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		FTM_EP_DATA_PTR	pData;

		xRet = FTM_LIST_getFirst(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
		if (xRet != FTM_RET_OK)
		{	
			WARN("Failed to get EP[%s] data\n", pReq->pEPID);	
			return	xRet;
		}
		memcpy(&pResp->xData, pData, sizeof(FTM_EP_DATA));
	}
	else
	{
		WARN("Failed to get EP[%s]\n", pReq->pEPID);	
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	pResp->nCount = 0;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_getDataList(pEP, pReq->nStartIndex, pResp->pData, pReq->nCount, &pResp->nCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP[%s] get data list error!\n", pReq->pEPID);
			pResp->nCount = 0;
		}
	}
	else
	{
		ERROR2(xRet, "EP[%s] is not found!\n", pReq->pEPID);
		pResp->nCount = 0;
	}

	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_DATA) * pResp->nCount;
	pResp->xCmd = pReq->xCmd;
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EP_getDataCount(pEP, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_type
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_TYPE_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_TYPE_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EP_get(pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xRet = FTOM_EP_getDataType(pEP, &pResp->xType);
	}

	return	pResp->xRet;
}

/********************************************************************
 *
 ********************************************************************/
FTM_RET	FTOM_SERVER_TRIGGER_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_create(&pReq->xTrigger, FTM_TRUE, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create trigger[%s].\n", pReq->xTrigger.pID);
		goto finish;
	}

	strcpy(pResp->pTriggerID, pTrigger->xInfo.pID);
	FTOM_LOG_createTrigger(&pTrigger->xInfo);

finish:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET	xRet;
	FTM_TRIGGER			xInfo;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_get(pReq->pTriggerID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to find trigger[%s].\n", pReq->pTriggerID);
		goto finish;
	}

	memcpy(&xInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));

	xRet = FTOM_TRIGGER_destroy(&pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove trigger[%s].\n", pReq->pTriggerID);
		goto finish;
	}

	FTOM_LOG_destroyTrigger(&xInfo);

finish:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_TRIGGER_count(&pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_get(pReq->pTriggerID, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_getAt(pReq->ulIndex, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_get(pReq->pTriggerID, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_TRIGGER_setInfo( pTrigger, pReq->xFields, &pReq->xTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Trigger[%s] failed to set info.\n", pReq->pTriggerID);
		}
	}
	else
	{
		ERROR2(xRet, "Trigger[%s] is not found.!\n", pReq->pTriggerID);	
	}

	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_create(&pReq->xAction, FTM_TRUE, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create action.\n");
		goto finish;
	}

	strcpy(pResp->pActionID, pAction->xInfo.pID);
	FTOM_LOG_createAction(&pAction->xInfo);

finish:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET	xRet;
	FTM_ACTION		xInfo;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_get(pReq->pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to find action[%s].\n", pReq->pActionID);
		goto finish;
	}

	xRet = FTOM_ACTION_destroy(&pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete the action[%s].\n", pReq->pActionID);
		goto finish;
	}

	FTOM_LOG_destroyAction(&xInfo);

finish:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_ACTION_count(&pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_get(pReq->pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get action[%s] info!\n", pAction->xInfo.pID);
		goto error;
	}

	memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));

error:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_getAt(pReq->ulIndex, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get action[%s] info!\n", pAction->xInfo.pID);
		goto error;
	}

	memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));

error:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction = NULL;

	xRet = FTOM_ACTION_get(pReq->pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get action[%s] info!\n", pAction->xInfo.pID);
		goto error;
	}

	xRet = FTOM_ACTION_setInfo(pAction, pReq->xFields, &pReq->xAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set action[%s] info!\n", pAction->xInfo.pID);
	}

error:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTOM_RULE_PTR	pRule = NULL;

	xRet = FTOM_RULE_create(&pReq->xRule, FTM_TRUE, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create rule[%s].\n", pReq->xRule.pID);
		goto finish;
	}

	strcpy(pResp->pRuleID, pRule->xInfo.pID);	

	xRet = FTOM_LOG_createRule(&pRule->xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create rule[%s] log!\n", pRule->xInfo.pID);	
		xRet = FTM_RET_OK;
	}

finish:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;


	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;
	FTM_RULE		xInfo;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULE_get(pReq->pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to find rule[%s]\n", pReq->pRuleID);
		goto finish;
	}

	memcpy(&xInfo, &pRule->xInfo, sizeof(FTM_RULE));

	xRet = FTOM_RULE_destroy(&pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete the rule[%s].\n", pReq->pRuleID);
		goto finish;
	}

	FTOM_LOG_destroyRule(&xInfo);

finish:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet	= xRet;

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_RULE_count(&pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	TRACE("Server Rule : %d\n", pResp->ulCount);
	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule = NULL;

	xRet = FTOM_RULE_get(pReq->pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get rule[%s].\n", pReq->pRuleID);
		goto error;
	}

	memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));

error:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule = NULL;

	xRet = FTOM_RULE_getAt(pReq->ulIndex, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get rule at [%lu].\n", pReq->ulIndex);
		goto error;
	}

	memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));

error:
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULE_get(pReq->pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get rule[%s].\n", pReq->pRuleID);
		goto error;
	}

	xRet = FTOM_RULE_setInfo(pRule, pReq->xFields, &pReq->xRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set rule[%s].\n", pReq->pRuleID);
	}

error:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	xRet;
}

static 
FTM_RET	FTOM_SERVER_LOG_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;

	xRet = FTOM_LOGGER_getAt(pReq->ulIndex, pReq->ulCount, pResp->pLogs, &pResp->ulCount);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(FTOM_RESP_LOG_GET_LIST_PARAMS) + sizeof(FTM_LOG) * pResp->ulCount;
	pResp->xRet = xRet;

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_LOG_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;

	xRet = FTOM_LOGGER_count(&pResp->ulCount);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(FTOM_RESP_LOG_COUNT_PARAMS);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_LOG_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_LOG_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_LOG_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;

	xRet = FTOM_LOGGER_remove(pReq->ulIndex, pReq->ulCount, &pResp->ulCount);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(FTOM_RESP_LOG_DEL_PARAMS);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

static
FTM_RET	FTOM_SERVER_DISCOVERY_start
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_START_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_START_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;

	xRet = FTOM_discoveryStart(pReq->pIP, pReq->usPort, pReq->ulRetryCount);

	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	xRet;
}

static
FTM_RET	FTOM_SERVER_DISCOVERY_getInfo
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_INFO_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_INFO_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;


	xRet = FTOM_discoveryIsFinished(&pResp->bFinished);
	if (xRet == FTM_RET_OK)
	{
		FTOM_SERVICE_PTR	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_DISCOVERY_getNodeInfoCount(pService->pData, &pResp->ulNodeCount);
			if (xRet == FTM_RET_OK)
			{
				xRet = FTOM_DISCOVERY_getEPInfoCount(pService->pData, &pResp->ulEPCount);
			}
		}
	}

	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	xRet;
}

static
FTM_RET	FTOM_SERVER_DISCOVERY_getNodeList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_NODE_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_NODE_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_SERVICE_PTR	pService;

	memset(pResp, 0, sizeof(*pResp));

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;

		for(i = 0 ; i < pReq->ulCount ; i++)
		{
			xRet = FTOM_DISCOVERY_getNodeInfoAt(pService->pData, pReq->ulIndex + i, &pResp->pNodeList[i]);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			pResp->ulCount++;
		}

		xRet = FTM_RET_OK;
	}

	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_NODE) * pResp->ulCount;

	return	xRet;
}

static
FTM_RET	FTOM_SERVER_DISCOVERY_getEPList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_DISCOVERY_GET_EP_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTOM_SERVICE_PTR	pService;
	FTM_ULONG			nMaxCount;

	memset(pResp, 0, sizeof(*pResp));

	nMaxCount = (ulRespLen - sizeof(FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS)) / sizeof(FTM_EP);
	if (nMaxCount <= 0)
	{
		xRet = FTM_RET_BUFFER_TOO_SMALL;
		goto finish;
	}


	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;

		for(i = 0 ; i < pReq->ulCount && pResp->ulCount < nMaxCount; i++)
		{
			xRet = FTOM_DISCOVERY_getEPInfoAt(pService->pData, pReq->ulIndex + i, &pResp->pEPList[i]);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			pResp->ulCount++;
		}

		xRet = FTM_RET_OK;
	}

finish:
	pResp->xRet = xRet;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP) * pResp->ulCount;

	return	xRet;
}

FTM_RET FTOM_SERVER_loadConfig
(
	FTOM_SERVER_PTR	pServer,
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xServer;

	xRet = FTM_CONFIG_getItem(pConfig, "server", &xServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Server configuration not found!\n");
		goto finish;
	}

	FTM_CONFIG_ITEM_getItemULONG(&xServer, "max_session", &pServer->xConfig.ulMaxSession);
	FTM_CONFIG_ITEM_getItemUSHORT(&xServer, "port", 		&pServer->xConfig.usPort);
	FTM_CONFIG_ITEM_getItemString(&xServer, "sm_key_file", pServer->xConfig.xSM.pKeyFile, FTM_FILE_NAME_LEN);

finish:
	return	xRet;
}

FTM_RET FTOM_SERVER_loadConfigFromFile
(
	FTOM_SERVER_PTR	pServer,
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pServer != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pRoot;

	xRet =FTM_CONFIG_create(pFileName, &pRoot, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SERVER configuration file[%s] load failed\n", pFileName);
		return	xRet;	
	}

	xRet = FTOM_SERVER_loadConfig(pServer, pRoot);

	FTM_CONFIG_destroy(&pRoot);

	return	xRet;
}

FTM_RET FTOM_SERVER_saveConfig
(
	FTOM_SERVER_PTR	pServer,
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xServer;

	xRet = FTM_CONFIG_getItem(pConfig, "server", &xServer);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "server", &xServer);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Server configuration not found!\n");
			goto finish;
		}
	}

	FTM_CONFIG_ITEM_setItemULONG(&xServer, 	"max_session", 	pServer->xConfig.ulMaxSession);
	FTM_CONFIG_ITEM_setItemUSHORT(&xServer, "port", 		pServer->xConfig.usPort);
	FTM_CONFIG_ITEM_setItemString(&xServer, "sm_key_file", 	pServer->xConfig.xSM.pKeyFile);

finish:
	return	xRet;
}

FTM_RET FTOM_SERVER_showConfig
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "Port", pServer->xConfig.usPort);
	MESSAGE("%16s : %lu\n", "Max Session", pServer->xConfig.ulMaxSession);
	MESSAGE("%16s : %s",	"SMKey File", pServer->xConfig.xSM.pKeyFile);

	return	FTM_RET_OK;
}
