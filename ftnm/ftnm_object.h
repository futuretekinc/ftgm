#ifndef	__FTNM_OBJECT_H__
#define	__FTNM_OBJECT_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

typedef	struct _FTNM_NODE _PTR_ FTNM_NODE_PTR;
typedef	struct 
{
	FTM_EP_INFO		xInfo;
		
	FTNM_NODE_PTR 	pNode;
	
	time_t			xTime;
	FTM_VALUE		xValue;
}	FTNM_EP, _PTR_ FTNM_EP_PTR;

typedef	struct
{
	FTNM_EP			xCommon;
}	FTNM_EP_SNMP, _PTR_ FTNMP_EP_SNMP_PTR;

#define	FTNM_SNMP_STATUS_RUNNING		0x00000001
#define	FTNM_SNMP_STATUS_COMPLETED		0x00000010

typedef	struct
{
	FTM_ULONG				nStatus;
	FTM_LIST				xEPList;
	FTNM_EP_PTR				pCurrentEP;
	struct snmp_session 	xSession;		/* SNMP session data */
	struct snmp_session 	*pSession;		/* SNMP session data */
}	FTNM_SNMPC, _PTR_ FTNM_SNMPC_PTR;

typedef	FTM_ULONG		FTNM_NODE_STATE;

#define	FTNM_NODE_STATE_CREATING 				0x00000001
#define	FTNM_NODE_STATE_CREATED					0x00000002
#define	FTNM_NODE_STATE_INITIALIZING			0x00000003
#define	FTNM_NODE_STATE_INITIALIZED				0x00000004
#define	FTNM_NODE_STATE_SYNCING					0x00000005
#define	FTNM_NODE_STATE_SYNCHRONIZED			0x00000006
#define	FTNM_NODE_STATE_EP_SCHEDULED			0x00000007
#define	FTNM_NODE_STATE_CALLED					0x00000008
#define	FTNM_NODE_STATE_WAITING					0x00000009
#define	FTNM_NODE_STATE_PROCESS_INIT			0x0000001B
#define	FTNM_NODE_STATE_PROCESSING				0x0000000B
#define	FTNM_NODE_STATE_PROCESS_FINISHED		0x0000000C
#define	FTNM_NODE_STATE_PAUSED					0x0000000D
#define	FTNM_NODE_STATE_INACTIVATED				0x0000000E
#define	FTNM_NODE_STATE_FINISHING				0x0000000F
#define	FTNM_NODE_STATE_FINISHED				0x00000010

typedef	struct _FTNM_NODE
{
	FTM_NODE_TYPE		xType;
	FTM_NODE_INFO		xInfo;
	FTM_LIST			xEPList;

	pthread_t			xPThread;
	pthread_mutex_t		xMutexLock;
	FTNM_NODE_STATE		xState;
	FTM_ULONG			ulRetry;
	time_t				xTimeout;
	FTM_LIST			xTaskList;
}	FTNM_NODE, _PTR_ FTNM_NODE_PTR;

typedef	struct 
{
	FTNM_NODE			xCommon;
	FTNM_SNMPC			xSNMPC;
}	FTNM_NODE_SNMPC, _PTR_ FTNM_NODE_SNMPC_PTR;

typedef	enum
{
	FTNM_NODE_TASK_CMD_PROCESS_EP = 1	
} FTNM_NODE_TASK_CMD, _PTR_ FTNM_NODE_TASK_CMD_PTR;

typedef	struct	
{
	FTNM_NODE_TASK_CMD	xCmd;
}	FTNM_NODE_TASK, _PTR_ FTNM_NODE_TASK_PTR;

#endif
