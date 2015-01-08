#ifndef	__FTDM_SERVER_CONFIG_H__
#define	__FTDM_SERVER_CONFIG_H__

#include "ftm_types.h"
#include "libconfig.h"
#include "simclist.h"

#define	FTDM_TYPE_NAME_LEN				64
#define	FTDM_FILE_PATH_LEN				256
#define	FTDM_FILE_NAME_LEN				256
#define	FTDM_SERVER_DEFAULT_PORT		8888
#define	FTDM_SERVER_DEFAULT_MAX_SESSION	10

typedef	struct
{
	FTM_ULONG	ulType;
	FTM_CHAR	pName[FTDM_TYPE_NAME_LEN];
}	FTDM_EP_TYPE_INFO, _PTR_ FTDM_EP_TYPE_INFO_PTR;

typedef	struct
{
	FTM_CHAR_PTR	pFileName;
}	FTDM_DB_CONFIG, _PTR_ FTDM_DB_CONFIG_PTR;

typedef	struct
{
	list_t				xInfos;
}	FTDM_EP_INFO_CONFIG, _PTR_ FTDM_EP_INFO_CONFIG_PTR;
typedef	struct
{
	list_t				xInfos;
}	FTDM_EP_CLASS_INFO_CONFIG, _PTR_ FTDM_EP_CLASS_INFO_CONFIG_PTR;

typedef	struct
{
	FTDM_DB_CONFIG				xDatabase;
	FTDM_EP_INFO_CONFIG			xEPInfo;
	FTDM_EP_CLASS_INFO_CONFIG	xEPClassInfo;
}	FTDM_CONFIG, _PTR_ FTDM_CONFIG_PTR;

typedef	struct
{
	struct 
	{
		FTM_CHAR_PTR	pFileName;
	}	xApp;

	struct
	{
		FTM_USHORT		usPort;
		FTM_ULONG		ulMaxSession;
	}	xNetwork;

	struct
	{	
		FTM_ULONG		ulPrintOutMode;
		struct 
		{
			FTM_CHAR_PTR	pFileName;
		} xTrace;

		struct 
		{
			FTM_CHAR_PTR	pFileName;
		} xError;
	}	xDebug;
	
	FTDM_CONFIG			xConfig;
}	FTDM_SERVER_CONFIG, _PTR_ FTDM_SERVER_CONFIG_PTR;

FTM_RET	FTDM_initServerConfig(FTDM_SERVER_CONFIG_PTR pConfig);
FTM_RET FTDM_loadServerConfig(FTDM_SERVER_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_destroyServerConfig(FTDM_SERVER_CONFIG_PTR pConfig);
FTM_RET	FTDM_showServerConfig(FTDM_SERVER_CONFIG_PTR pConfig);

FTM_RET FTDM_initDBConfig(FTDM_DB_CONFIG_PTR pConfig);
FTM_RET FTDM_loadDBConfig(FTDM_DB_CONFIG_PTR pDBConfig, config_t pConfig);
FTM_RET FTDM_destroyDBConfig(FTDM_DB_CONFIG_PTR pConfig);

FTM_RET	FTDM_initEPInfoConfig(FTDM_EP_INFO_CONFIG_PTR pConfig);
FTM_RET FTDM_loadEPInfoConfig(FTDM_EP_INFO_CONFIG_PTR pEPInfoConfig, config_t pConfig);
FTM_RET FTDM_destroyEPInfoConfig(FTDM_EP_INFO_CONFIG_PTR pConfig);

FTM_RET FTDM_initEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig);
FTM_RET FTDM_loadEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pEPClassInfoConfig, config_t pConfig);
FTM_RET FTDM_destroyEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig);
FTM_RET FTDM_showEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig);
#endif
