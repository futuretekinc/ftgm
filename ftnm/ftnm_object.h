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

typedef	enum
{
	FTNM_NODE_TASK_CMD_PROCESS_EP = 1	
} FTNM_NODE_TASK_CMD, _PTR_ FTNM_NODE_TASK_CMD_PTR;

typedef	struct	
{
	FTNM_NODE_TASK_CMD	xCmd;
}	FTNM_NODE_TASK, _PTR_ FTNM_NODE_TASK_PTR;

#endif
