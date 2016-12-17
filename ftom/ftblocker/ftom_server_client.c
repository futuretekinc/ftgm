#include "ftom_blocker.h"
#include "ftom_server_client.h"
#include "ftom_net_client.h"

static
FTM_RET	FTOM_DUMMY_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_destroy
(
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_init
(
	FTOM_SERVER_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_final
(
	FTOM_SERVER_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_start
(
	FTOM_SERVER_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_stop
(
	FTOM_SERVER_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_isRunning
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
);

static
FTM_RET	FTOM_DUMMY_CLIENT_waitingForFinished
(
	FTOM_SERVER_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_load
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_save
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_show
(
	FTOM_SERVER_CLIENT_PTR	pClient
);


static
FTM_RET	FTOM_DUMMY_CLIENT_setNotifyCB
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

static
FTM_RET FTOM_DUMMY_CLIENT_MESSAGE_send
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
);

static
FTM_RET	FTOM_DUMMY_CLIENT_MESSAGE_process
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
);

static
FTOM_SERVER_CLIENT_MODULE	_pModules[] =
{
	{
		.pName	= "default",
		
		.fCreate 			= (FTOM_SERVER_CLIENT_CREATE)FTOM_DUMMY_CLIENT_create,
		.fDestroy			= (FTOM_SERVER_CLIENT_DESTROY)FTOM_DUMMY_CLIENT_destroy,
		.fInit 				= (FTOM_SERVER_CLIENT_INIT)FTOM_DUMMY_CLIENT_init,
		.fFinal				= (FTOM_SERVER_CLIENT_FINAL)FTOM_DUMMY_CLIENT_final,
		.fStart				= (FTOM_SERVER_CLIENT_START)FTOM_DUMMY_CLIENT_start,
		.fStop 				= (FTOM_SERVER_CLIENT_STOP)FTOM_DUMMY_CLIENT_stop,
		.fIsRunning			= (FTOM_SERVER_CLIENT_IS_RUNNING)FTOM_DUMMY_CLIENT_isRunning,
		.fWaitingForFinished= (FTOM_SERVER_CLIENT_WAITING_FOR_FINISHED)FTOM_DUMMY_CLIENT_waitingForFinished,
		.fConfigLoad 		= (FTOM_SERVER_CLIENT_CONFIG_LOAD)FTOM_DUMMY_CLIENT_CONFIG_load,
		.fConfigSave 		= (FTOM_SERVER_CLIENT_CONFIG_SAVE)FTOM_DUMMY_CLIENT_CONFIG_save,
		.fConfigShow 		= (FTOM_SERVER_CLIENT_CONFIG_SHOW)FTOM_DUMMY_CLIENT_CONFIG_show,
		.fSetNotifyCB		= (FTOM_SERVER_CLIENT_SET_NOTIFY_CB)FTOM_DUMMY_CLIENT_setNotifyCB,
		.fMessageSend		= (FTOM_SERVER_CLIENT_MESSAGE_SEND)FTOM_DUMMY_CLIENT_MESSAGE_send,
		.fMessageProcess	= (FTOM_SERVER_CLIENT_MESSAGE_PROCESS)FTOM_DUMMY_CLIENT_MESSAGE_process
	},
	{
		.pName	= "netclient",

		.fCreate 			= (FTOM_SERVER_CLIENT_CREATE)FTOM_NET_CLIENT_create,
		.fDestroy			= (FTOM_SERVER_CLIENT_DESTROY)FTOM_NET_CLIENT_destroy,
		.fInit 				= (FTOM_SERVER_CLIENT_INIT)FTOM_NET_CLIENT_init,
		.fFinal				= (FTOM_SERVER_CLIENT_FINAL)FTOM_NET_CLIENT_final,
		.fStart				= (FTOM_SERVER_CLIENT_START)FTOM_NET_CLIENT_start,
		.fStop 				= (FTOM_SERVER_CLIENT_STOP)FTOM_NET_CLIENT_stop,
		.fIsRunning			= (FTOM_SERVER_CLIENT_IS_RUNNING)FTOM_NET_CLIENT_isRunning,
		.fWaitingForFinished= (FTOM_SERVER_CLIENT_WAITING_FOR_FINISHED)FTOM_NET_CLIENT_waitingForFinished,
		.fConfigLoad 		= (FTOM_SERVER_CLIENT_CONFIG_LOAD)FTOM_NET_CLIENT_CONFIG_load,
		.fConfigSave 		= (FTOM_SERVER_CLIENT_CONFIG_SAVE)FTOM_NET_CLIENT_CONFIG_save,
		.fConfigShow 		= (FTOM_SERVER_CLIENT_CONFIG_SHOW)FTOM_NET_CLIENT_CONFIG_show,
		.fSetNotifyCB		= (FTOM_SERVER_CLIENT_SET_NOTIFY_CB)FTOM_NET_CLIENT_setNotifyCB,
		.fMessageSend		= (FTOM_SERVER_CLIENT_MESSAGE_SEND)FTOM_NET_CLIENT_MESSAGE_send,
		.fMessageProcess	= (FTOM_SERVER_CLIENT_MESSAGE_PROCESS)FTOM_NET_CLIENT_MESSAGE_process
	},
};


FTM_RET	FTOM_SERVER_CLIENT_getModule
(
	FTM_CHAR_PTR	pName,
	FTOM_SERVER_CLIENT_MODULE_PTR _PTR_ ppModule
)
{
	ASSERT(pName != NULL);
	ASSERT(ppModule != NULL);
	FTM_INT	i;

	for(i = 0 ; i < sizeof(_pModules) / sizeof(FTOM_SERVER_CLIENT_MODULE) ; i++)
	{
		if (strcasecmp(_pModules[i].pName, pName) == 0)
		{
			*ppModule = &_pModules[i];	

			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTOM_DUMMY_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_destroy
(
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_init
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_final
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_start
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_stop
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_isRunning
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_waitingForFinished
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_load
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_save
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_show
(
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


FTM_RET	FTOM_DUMMY_CLIENT_setNotifyCB
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET FTOM_DUMMY_CLIENT_MESSAGE_send
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_MESSAGE_process
(
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


