#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"

struct FTDM_DBIF_STRUCT ;
struct FTDM_MODEM_STRUCT;
struct FTDM_EPM_STRUCT;
struct FTDM_LOGGER_STRUCT;

typedef struct FTDM_CONTEXT_STRUCT
{
	struct FTDM_DBIF_STRUCT _PTR_	pDBIF;
	struct FTDM_NODEM_STRUCT _PTR_	pNodeM;	
	struct FTDM_EPM_STRUCT _PTR_	pEPM;	
	struct FTDM_LOGGER_STRUCT _PTR_	pLogger;
} FTDM_CONTEXT, _PTR_ FTDM_CONTEXT_PTR;

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
#include "ftdm_sqlite.h"

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
	FTM_CONFIG_PTR 		pConfig
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

