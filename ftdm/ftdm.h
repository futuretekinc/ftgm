#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"
#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node.h"
#include "ftdm_node_management.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
#include "ftdm_ep_class.h"
#include "ftdm_server.h"
#include "ftdm_logger.h"
#include "ftdm_modules.h"

typedef struct FTDM_CONTEXT_STRUCT
{
	FTDM_NODEM_PTR	pNodeM;	
	FTDM_EPM_PTR	pEPM;	
	FTDM_LOGGER_PTR	pLogger;
} FTDM_CONTEXT, _PTR_ FTDM_CONTEXT_PTR;

FTM_RET	FTDM_init
(
	FTDM_CONTEXT_PTR	pFTDM
);

FTM_RET	FTDM_final
(
	FTDM_CONTEXT_PTR	pFTDM
);

FTM_RET FTDM_loadConfig
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTDM_CFG_PTR 		pConfig
);

FTM_RET	FTDM_loadFromFile
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTDM_loadObjectFromFile
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTDM_saveObjectToDB
(
	FTDM_CONTEXT_PTR	pFTDM
);

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
);

FTM_RET	FTDM_getServer
(
	FTDM_SERVER_PTR _PTR_ ppServer
);
	
FTM_RET	FTDM_removeInvalidData
(
	FTDM_CONTEXT_PTR	pDM
);	
#endif

