#ifndef	__FTNM_OBJECT_H__
#define	__FTNM_OBJECT_H__

typedef	struct _FTNM_NODE _PTR_ FTNM_NODE_PTR;
typedef	struct 
{
	FTM_EP_TYPE		xType;
	FTM_EP_INFO		xInfo;

	FTNM_NODE_PTR 	pNode;
}	FTNM_EP, _PTR_ FTNM_EP_PTR;

typedef	struct
{
	FTNM_EP			xCommon;	
}	FTNM_EP_SNMP, _PTR_ FTNMP_EP_SNMP_PTR;

#define	FTNM_NODE_STATE_STOP	0
#define	FTNM_NODE_STATE_RUN		1

typedef	struct _FTNM_NODE
{
	FTM_NODE_TYPE		xType;
	FTM_NODE_INFO		xInfo;
	FTM_LIST			xEPList;

	pthread_t			xPThread;
	pthread_mutex_t		xMutexLock;
	FTM_ULONG			xState;
}	FTNM_NODE, _PTR_ FTNM_NODE_PTR;

typedef	struct
{
	FTNM_NODE			xCommon;	
}	FTNM_NODE_SNMP, _PTR_ FTNMP_NODE_SNMP_PTR;
#endif
