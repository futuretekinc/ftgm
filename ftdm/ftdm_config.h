#ifndef	__FTDM_CONFIG_H__
#define	__FTDM_CONFIG_H__

#include "ftm_types.h"
#include "libconfig.h"
#include "ftm_list.h"
#include "ftm_debug.h"
#include "ftm_ep.h"

#define	FTDM_SERVER_DEFAULT_PORT		8888
#define	FTDM_SERVER_DEFAULT_MAX_SESSION	10

typedef	struct
{
	FTM_USHORT				usPort;
	FTM_ULONG				ulMaxSession;
}	FTDM_CFG_SERVER, _PTR_ FTDM_CFG_SERVER_PTR;

typedef	struct
{
	FTM_CHAR_PTR			pFileName;
	FTM_ULONG				ulLiftTime;
}	FTDM_CFG_DB, _PTR_ FTDM_CFG_DB_PTR;

typedef	struct
{
	FTM_LIST				xList;
}	FTDM_CFG_NODE, _PTR_ FTDM_CFG_NODE_PTR;

typedef	struct
{
	FTM_LIST				xList;
	FTM_LIST				xClassList;
}	FTDM_CFG_EP, _PTR_ FTDM_CFG_EP_PTR;

typedef	struct
{
	FTDM_CFG_SERVER			xServer;
	FTDM_CFG_DB				xDB;
	FTDM_CFG_NODE			xNode;
	FTDM_CFG_EP				xEP;
	FTM_PRINT_CFG			xPrint;
}	FTDM_CFG, _PTR_ FTDM_CFG_PTR;

FTM_RET	FTDM_CFG_init(FTDM_CFG_PTR pConfig);
FTM_RET	FTDM_CFG_load(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_CFG_final(FTDM_CFG_PTR pConfig);

FTM_RET FTDM_CFG_show(FTDM_CFG_PTR pConfig);

FTM_RET FTDM_CFG_setDBFileName(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);

FTM_RET	FTDM_CFG_setServer(FTDM_CFG_PTR pConfig, FTM_SERVER_INFO_PTR pInfo);

FTM_RET	FTDM_CFG_NODE_INFO_append(FTDM_CFG_NODE_PTR pConfig, FTM_NODE_INFO_PTR pInfo);
FTM_RET FTDM_CFG_NODE_INFO_count(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET	FTDM_CFG_NODE_INFO_getAt(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo);

FTM_RET	FTDM_CFG_EP_INFO_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_INFO_PTR pInfo);
FTM_RET FTDM_CFG_EP_INFO_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET	FTDM_CFG_EP_INFO_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo);

FTM_RET FTDM_CFG_EP_CLASS_INFO_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS_INFO_PTR pInfo);
FTM_RET FTDM_CFG_EP_CLASS_INFO_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET FTDM_CFG_EP_CLASS_INFO_get(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo);
FTM_RET FTDM_CFG_EP_CLASS_INFO_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo);

FTM_CHAR_PTR	FTDM_CFG_EP_getTypeString(FTM_ULONG ulType);
FTM_CHAR_PTR	FTDM_CFG_SNMP_getVersionString(FTM_ULONG ulVersion);

#endif
