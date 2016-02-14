#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include "ftnm.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_snmptrapd.h"

typedef	FTM_ULONG		FTNM_SNMPC_STATE, _PTR_ FTNM_SNMPC_STATE_PTR;

#define	FTNM_SNMPC_STATE_UNKNOWN		0x00000000
#define	FTNM_SNMPC_STATE_INITIALIZED	0x00000001
#define	FTNM_SNMPC_STATE_RUNNING		0x00000002
#define	FTNM_SNMPC_STATE_TIMEOUT		0x00000003
#define	FTNM_SNMPC_STATE_ERROR			0x00000004
#define	FTNM_SNMPC_STATE_COMPLETED		0x00000005
/*
#define	FTNM_SNMPC_EP_TYPE_TEMPERATURE	0x00000001
#define	FTNM_SNMPC_EP_TYPE_HUMIDITY		0x00000002
#define	FTNM_SNMPC_EP_TYPE_VOLTAGE		0x00000003
#define	FTNM_SNMPC_EP_TYPE_CURRENT		0x00000004
#define	FTNM_SNMPC_EP_TYPE_DI			0x00000005
#define	FTNM_SNMPC_EP_TYPE_DO			0x00000006
#define	FTNM_SNMPC_EP_TYPE_GAS			0x00000007
#define	FTNM_SNMPC_EP_TYPE_POWER		0x00000008
#define	FTNM_SNMPC_EP_TYPE_COUNT		0x00000009
#define	FTNM_SNMPC_EP_TYPE_PRESSURE		0x0000000A
#define	FTNM_SNMPC_EP_TYPE_DISCRETE		0x0000000B
#define	FTNM_SNMPC_EP_TYPE_DEVICE		0x0000000C
*/
typedef	struct
{
	FTM_CHAR		pName[1024];
	FTM_LIST		xMIBList;

	FTM_ULONG		ulMaxRetryCount;
}	FTNM_SNMPC_CONFIG, _PTR_ FTNM_SNMPC_CONFIG_PTR;

typedef	struct 
{
	FTNM_SNMPC_CONFIG	xConfig;
	FTNM_SNMPTRAPD		xTrapd;
	pthread_t			xPThread;
	pthread_t			xTrapD;
}	FTNM_SNMPC, _PTR_ FTNM_SNMPC_PTR;

FTM_RET	FTNM_SNMPC_init(void);
FTM_RET	FTNM_SNMPC_final(void);

FTM_RET FTNM_SNMPC_run(void);

FTM_RET	FTNM_SNMPC_initConfig(void);
FTM_RET	FTNM_SNMPC_finalConfig(void);
FTM_RET FTNM_SNMPC_loadConfig(FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_SNMPC_showConfig(void);


FTM_ULONG	FTNM_SNMPC_getMaxRetryCount(void);
FTM_RET		FTNM_SNMPC_getEPData(FTNM_NODE_SNMPC_PTR pNode, FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

FTM_RET		FTNM_SNMPC_getOID(FTM_CHAR_PTR pInput, oid *pOID, size_t *pnOIDLen);

#endif

