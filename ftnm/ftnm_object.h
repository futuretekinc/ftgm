#ifndef	__FTNM_OBJECT_H__
#define	__FTNM_OBJECT_H__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

typedef	struct _FTNM_NODE _PTR_ FTNM_NODE_PTR;
typedef	struct 
{
	FTM_EP_INFO		xInfo;
	FTNM_NODE_PTR 	pNode;
	FTM_EP_DATA		xData;
	union
	{
		struct	
		{
			oid		pOID[MAX_OID_LEN];
			size_t	nOIDLen;
		}	xSNMP;
	}	xOption;
}	FTNM_EP, _PTR_ FTNM_EP_PTR;

#define	FTNM_SNMPC_STATE_UNKNOWN		0x00000000
#define	FTNM_SNMPC_STATE_INITIALIZED	0x00000001
#define	FTNM_SNMPC_STATE_COMPLETED		0x00000002
#define	FTNM_SNMPC_STATE_RUNNING		0x00000003
#define	FTNM_SNMPC_STATE_TIMEOUT		0x00000004
#define	FTNM_SNMPC_STATE_ERROR			0x00000005

typedef	struct
{
	FTM_ULONG				nState;
	FTM_LIST				xEPList;
	FTNM_EP_PTR				pCurrentEP;
	struct snmp_session 	*pSession;		/* SNMP session data */
	time_t					xTimeout;
	struct 
	{
		
		FTM_ULONG				ulRequest;
		FTM_ULONG				ulResponse;
	}	xStatistics;	
}	FTNM_SNMPC, _PTR_ FTNM_SNMPC_PTR;

typedef	FTM_ULONG		FTNM_NODE_STATE;

#define	FTNM_NODE_STATE_CREATING 				0x00000001
#define	FTNM_NODE_STATE_CREATED					0x00000002
#define	FTNM_NODE_STATE_INITIALIZING			0x00000003
#define	FTNM_NODE_STATE_INITIALIZED				0x00000004
#define	FTNM_NODE_STATE_SYNCING					0x00000005
#define	FTNM_NODE_STATE_SYNCHRONIZED			0x00000006
#define	FTNM_NODE_STATE_PROCESS_INIT			0x0000001B
#define	FTNM_NODE_STATE_RUN						0x0000001C
#define	FTNM_NODE_STATE_RUNNING					0x0000000B
#define	FTNM_NODE_STATE_PROCESS_FINISHED		0x0000000C
#define	FTNM_NODE_STATE_FINISHED				0x00000010
#define	FTNM_NODE_STATE_ABORT					0x00000001

typedef	struct _FTNM_NODE
{
	FTM_NODE_TYPE		xType;
	FTM_NODE_INFO		xInfo;
	FTM_LIST			xEPList;

	pthread_t			xPThread;
	pthread_mutex_t		xMutexLock;
	FTNM_NODE_STATE		xState;
	FTM_ULONG			ulRetry;
	int64_t				xTimeout;
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
