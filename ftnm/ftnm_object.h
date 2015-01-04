#ifndef	__FTNM_OBJECT_H__
#define	__FTNM_OBJECT_H__

#include "ftm_types.h"
#include "ftdm_type.h"
#include "simclist.h"
#include <pthread.h>
#include "ftnm_snmp_client.h"

typedef	enum
{
	FTNM_NODE_TYPE_SNMP = 0
}	FTNM_NODE_TYPE, _PTR_ FTNM_NODE_TYPE_PTR;
	
typedef	struct _FTNM_NODE
{
	FTDM_DEVICE_INFO	xInfo;
	FTNM_NODE_TYPE		xType;
	list_t				xEPList;
	pthread_t			xPThread;
	FTM_INT				nUpdateInterval;
}	FTNM_NODE, _PTR_ FTNM_NODE_PTR;

typedef	struct _FTNM_NODE_SNMP
{
	FTDM_DEVICE_INFO	xInfo;
	FTNM_NODE_TYPE		xType;
	list_t				xEPList;
	pthread_t			xPThread;
	FTM_INT				nUpdateInterval;
	FTNM_SNMP_INFO		xSNMP;
}	FTNM_NODE_SNMP, _PTR_ FTNM_NODE_SNMP_PTR;

FTM_RET FTNM_initNodeManager(void);
FTM_RET FTNM_finalNodeManager(void);

FTM_RET	FTNM_createNodeSNMP(FTDM_DEVICE_INFO_PTR pInfo);

FTM_RET	FTNM_destroyNode(FTM_CHAR_PTR	pDID);
FTM_RET FTNM_getNode(FTDM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode);

FTM_RET FTNM_addEP(FTDM_EP_INFO_PTR pInfo);

FTM_RET	FTNM_startNode(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_stopNode(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_restartNode(FTNM_NODE_PTR pNode);
#endif

