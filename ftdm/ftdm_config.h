#ifndef	__FTDM_CONFIG_H__
#define	__FTDM_CONFIG_H__

#include "ftm_types.h"
#include "libconfig.h"
#include "simclist.h"

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
}	FTDM_CFG_DB, _PTR_ FTDM_CFG_DB_PTR;

typedef	struct
{
	list_t					xList;
}	FTDM_CFG_NODE, _PTR_ FTDM_CFG_NODE_PTR;

typedef	struct
{
	list_t					xList;
	list_t					xClassList;
}	FTDM_CFG_EP, _PTR_ FTDM_CFG_EP_PTR;

typedef	struct
{
	FTDM_CFG_SERVER			xServer;
	FTDM_CFG_DB				xDB;
	FTDM_CFG_NODE			xNode;
	FTDM_CFG_EP				xEP;
}	FTDM_CFG, _PTR_ FTDM_CFG_PTR;

FTM_RET	FTDM_CFG_init(FTDM_CFG_PTR pConfig);
FTM_RET	FTDM_CFG_load(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_CFG_final(FTDM_CFG_PTR pConfig);

FTM_RET FTDM_CFG_show(FTDM_CFG_PTR pConfig);

FTM_RET FTDM_CFG_setDBFileName(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);

FTM_RET	FTDM_CFG_setServer(FTDM_CFG_PTR pConfig, FTM_SERVER_INFO_PTR pInfo);

FTM_RET	FTDM_CFG_addNodeInfo(FTDM_CFG_NODE_PTR pConfig, FTM_NODE_INFO_PTR pInfo);
FTM_RET FTDM_CFG_getNodeInfoCount(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET	FTDM_CFG_getNodeInfoByIndex(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo);

FTM_RET	FTDM_CFG_addEPInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_INFO_PTR pInfo);
FTM_RET FTDM_CFG_getEPInfoCount(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET	FTDM_CFG_getEPInfoByIndex(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo);

FTM_RET FTDM_CFG_addEPClassInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS_INFO_PTR pInfo);
FTM_RET FTDM_CFG_getEPClassInfoCount(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount);
FTM_RET FTDM_CFG_getEPClassInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo);
FTM_RET FTDM_CFG_getEPClassInfoByIndex(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo);
#endif
