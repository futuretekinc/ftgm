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
	struct 
	{
		FTM_CHAR	pFileName[FTDM_FILE_NAME_LEN];
	}	xApp;

	struct
	{
		FTM_CHAR	pFileName[FTDM_FILE_NAME_LEN];
	}	xDatabase;

	struct
	{
		FTM_USHORT	usPort;
		FTM_ULONG	ulMaxSession;
	}	xNetwork;

	struct
	{
		list_t		xInfoList;				
		list_t		xClassInfoList;
	}	xEP;

	struct
	{	
		FTM_ULONG	ulPrintOutMode;
		struct 
		{
			FTM_CHAR	pPath[FTDM_FILE_PATH_LEN];
			FTM_CHAR	pPrefix[FTDM_FILE_NAME_LEN];
		} xTrace;

		struct 
		{
			FTM_CHAR	pPath[FTDM_FILE_PATH_LEN];
			FTM_CHAR	pPrefix[FTDM_FILE_NAME_LEN];
		} xError;

	}	xDebug;
		
}	FTDM_SERVER_CONFIG, _PTR_ FTDM_SERVER_CONFIG_PTR;

FTM_RET	FTDMS_initConfig(FTDM_SERVER_CONFIG_PTR pConfig);
FTM_RET FTDMS_loadConfig(FTDM_SERVER_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDMS_destroyConfig(FTDM_SERVER_CONFIG_PTR pConfig);
FTM_RET	FTDMS_showConfig(FTDM_SERVER_CONFIG_PTR pConfig);

FTM_RET	FTDMS_getEPClassInfo
(
	FTDM_SERVER_CONFIG_PTR			pConfig, 
	FTM_EPID 						xEPID, 
	FTM_EP_CLASS_INFO_PTR _PTR_ 	ppclassInfo
);

#endif
