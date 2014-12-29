#ifndef	__FTNM_OBJECT_H__
#define	__FTNM_OBJECT_H__

#include "ftm_types.h"
#include "ftdm_type.h"
#include "simclist.h"
#include <pthread.h>

typedef	struct _FTNM_NODE
{
	FTDM_DEVICE_INFO	xInfo;
	list_t				xEPList;
	pthread_t			xPThread;
}	FTNM_NODE, _PTR_ FTNM_NODE_PTR;

FTM_RET FTNM_initNodeManager(void);
FTM_RET FTNM_finalNodeManager(void);

FTM_RET	FTNM_createNode(FTDM_DEVICE_INFO_PTR pInfo);
FTM_RET	FTNM_destroyNode(FTM_CHAR_PTR	pDID);
FTM_RET FTNM_getNode(FTDM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode);

FTM_RET	FTNM_startNode(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_stopNode(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_restartNode(FTNM_NODE_PTR pNode);
#endif

