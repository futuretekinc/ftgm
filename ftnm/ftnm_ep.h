#ifndef	__FTNM_EP_H__
#define	__FTNM_EP_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftnm.h"
#include <pthread.h>
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

FTM_RET	FTNM_EP_init(FTNM_CONTEXT_PTR pCTX);
FTM_RET FTNM_EP_final(FTNM_CONTEXT_PTR pCTX);

FTM_RET	FTNM_EP_create(FTNM_CONTEXT_PTR pCTX, FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppNode);
FTM_RET	FTNM_EP_destroy(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR pEP);

FTM_RET	FTNM_EP_count(FTNM_CONTEXT_PTR pCTX, FTM_EP_CLASS xClass, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EP_getList
(
	FTNM_CONTEXT_PTR pCTX, 
	FTM_EP_CLASS 	xClass, 
	FTM_EPID_PTR 	pEPID, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTNM_EP_get(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP);
FTM_RET FTNM_EP_getAt(FTNM_CONTEXT_PTR pCTX, FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP);

FTM_RET	FTNM_EP_attach(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_EP_detach(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR pEP);

FTM_RET	FTNM_EP_DATA_info(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_DATA_count(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_DATA_set(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_EP_DATA_PTR pData);

#endif

