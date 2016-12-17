#include "ftom_cloud_client.h"
#include "ftom_azure_client.h"
#include "ftom_tp_client.h"

static
FTM_RET	FTOM_DUMMY_CLIENT_create
(
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_destroy
(
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_init
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_final
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_start
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_stop
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_isRunning
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
);

static
FTM_RET	FTOM_DUMMY_CLIENT_waitingForFinished
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_load
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_save
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

static
FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_show
(
	FTOM_CLOUD_CLIENT_PTR	pClient
);


static
FTM_RET	FTOM_DUMMY_CLIENT_setNotifyCB
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

static
FTM_RET FTOM_DUMMY_CLIENT_MESSAGE_send
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
);

static
FTM_RET	FTOM_DUMMY_CLIENT_MESSAGE_process
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
);

static
FTOM_CLOUD_CLIENT_MODULE	_pModules[] =
{
	{
		.pName	= "default",
		
		.xFlags	=
		{
			.bServerSync	=	FTM_FALSE,	
		},

		.fCreate 			= (FTOM_CLOUD_CLIENT_CREATE)FTOM_DUMMY_CLIENT_create,
		.fDestroy			= (FTOM_CLOUD_CLIENT_DESTROY)FTOM_DUMMY_CLIENT_destroy,
		.fInit 				= (FTOM_CLOUD_CLIENT_INIT)FTOM_DUMMY_CLIENT_init,
		.fFinal				= (FTOM_CLOUD_CLIENT_FINAL)FTOM_DUMMY_CLIENT_final,
		.fStart				= (FTOM_CLOUD_CLIENT_START)FTOM_DUMMY_CLIENT_start,
		.fStop 				= (FTOM_CLOUD_CLIENT_STOP)FTOM_DUMMY_CLIENT_stop,
		.fIsRunning			= (FTOM_CLOUD_CLIENT_IS_RUNNING)FTOM_DUMMY_CLIENT_isRunning,
		.fWaitingForFinished= (FTOM_CLOUD_CLIENT_WAITING_FOR_FINISHED)FTOM_DUMMY_CLIENT_waitingForFinished,
		.fConfigLoad 		= (FTOM_CLOUD_CLIENT_CONFIG_LOAD)FTOM_DUMMY_CLIENT_CONFIG_load,
		.fConfigSave 		= (FTOM_CLOUD_CLIENT_CONFIG_SAVE)FTOM_DUMMY_CLIENT_CONFIG_save,
		.fConfigShow 		= (FTOM_CLOUD_CLIENT_CONFIG_SHOW)FTOM_DUMMY_CLIENT_CONFIG_show,
		.fSetNotifyCB		= (FTOM_CLOUD_CLIENT_SET_NOTIFY_CB)FTOM_DUMMY_CLIENT_setNotifyCB,
		.fMessageSend		= (FTOM_CLOUD_CLIENT_MESSAGE_SEND)FTOM_DUMMY_CLIENT_MESSAGE_send,
		.fMessageProcess	= (FTOM_CLOUD_CLIENT_MESSAGE_PROCESS)FTOM_DUMMY_CLIENT_MESSAGE_process
	},
	{
		.pName	= "azclient",

		.xFlags	=
		{
			.bServerSync	=	FTM_FALSE
		},

		.fCreate 			= (FTOM_CLOUD_CLIENT_CREATE)FTOM_AZURE_CLIENT_create,
		.fDestroy			= (FTOM_CLOUD_CLIENT_DESTROY)FTOM_AZURE_CLIENT_destroy,
		.fInit 				= (FTOM_CLOUD_CLIENT_INIT)FTOM_AZURE_CLIENT_init,
		.fFinal				= (FTOM_CLOUD_CLIENT_FINAL)FTOM_AZURE_CLIENT_final,
		.fStart				= (FTOM_CLOUD_CLIENT_START)FTOM_AZURE_CLIENT_start,
		.fStop 				= (FTOM_CLOUD_CLIENT_STOP)FTOM_AZURE_CLIENT_stop,
		.fIsRunning			= (FTOM_CLOUD_CLIENT_IS_RUNNING)FTOM_AZURE_CLIENT_isRunning,
		.fWaitingForFinished= (FTOM_CLOUD_CLIENT_WAITING_FOR_FINISHED)FTOM_AZURE_CLIENT_waitingForFinished,
		.fConfigLoad 		= (FTOM_CLOUD_CLIENT_CONFIG_LOAD)FTOM_AZURE_CLIENT_CONFIG_load,
		.fConfigSave 		= (FTOM_CLOUD_CLIENT_CONFIG_SAVE)FTOM_AZURE_CLIENT_CONFIG_save,
		.fConfigShow 		= (FTOM_CLOUD_CLIENT_CONFIG_SHOW)FTOM_AZURE_CLIENT_CONFIG_show,
		.fSetNotifyCB		= (FTOM_CLOUD_CLIENT_SET_NOTIFY_CB)FTOM_AZURE_CLIENT_setNotifyCB,
		.fMessageSend		= (FTOM_CLOUD_CLIENT_MESSAGE_SEND)FTOM_AZURE_CLIENT_MESSAGE_send,
		.fMessageProcess	= (FTOM_CLOUD_CLIENT_MESSAGE_PROCESS)FTOM_AZURE_CLIENT_MESSAGE_process
	},
	{
		.pName	= "tpclient",

		.xFlags	=
		{
			.bServerSync	=	FTM_TRUE,	
		},

		.fCreate 			= (FTOM_CLOUD_CLIENT_CREATE)FTOM_TP_CLIENT_create,
		.fDestroy			= (FTOM_CLOUD_CLIENT_DESTROY)FTOM_TP_CLIENT_destroy,
		.fInit 				= (FTOM_CLOUD_CLIENT_INIT)FTOM_TP_CLIENT_init,
		.fFinal				= (FTOM_CLOUD_CLIENT_FINAL)FTOM_TP_CLIENT_final,
		.fStart				= (FTOM_CLOUD_CLIENT_START)FTOM_TP_CLIENT_start,
		.fStop 				= (FTOM_CLOUD_CLIENT_STOP)FTOM_TP_CLIENT_stop,
		.fIsRunning			= (FTOM_CLOUD_CLIENT_IS_RUNNING)FTOM_TP_CLIENT_isRunning,
		.fWaitingForFinished= (FTOM_CLOUD_CLIENT_WAITING_FOR_FINISHED)FTOM_TP_CLIENT_waitingForFinished,
		.fConfigLoad 		= (FTOM_CLOUD_CLIENT_CONFIG_LOAD)FTOM_TP_CLIENT_CONFIG_load,
		.fConfigSave 		= (FTOM_CLOUD_CLIENT_CONFIG_SAVE)FTOM_TP_CLIENT_CONFIG_save,
		.fConfigShow		= (FTOM_CLOUD_CLIENT_CONFIG_SHOW)FTOM_TP_CLIENT_CONFIG_show,
		.fSetNotifyCB		= (FTOM_CLOUD_CLIENT_SET_NOTIFY_CB)FTOM_TP_CLIENT_setNotifyCB,
		.fMessageSend		= (FTOM_CLOUD_CLIENT_MESSAGE_SEND)FTOM_TP_CLIENT_MESSAGE_send,
		.fMessageProcess	= (FTOM_CLOUD_CLIENT_MESSAGE_PROCESS)FTOM_TP_CLIENT_MESSAGE_process,
		.xGateway = 
		{
			.fUpdateStatus	= (FTOM_CLOUD_CLIENT_GATEWAY_UPDATE_STATUS)FTOM_TP_CLIENT_GATEWAY_updateStatus,
		},
		.xNode = 
		{
			.fIsExist		= (FTOM_CLOUD_CLIENT_NODE_IS_EXIST)FTOM_TP_CLIENT_NODE_isExist,
			.fGetCount		= (FTOM_CLOUD_CLIENT_NODE_GET_COUNT)FTOM_TP_CLIENT_NODE_getCount,
			.fRegister		= (FTOM_CLOUD_CLIENT_NODE_REGISTER)FTOM_TP_CLIENT_NODE_register,
		},
		.xEP = 
		{
			.fIsExist		= (FTOM_CLOUD_CLIENT_EP_IS_EXIST)FTOM_TP_CLIENT_EP_isExist,
			.fGetEPIDAt		= (FTOM_CLOUD_CLIENT_EP_GET_EPID_AT)FTOM_TP_CLIENT_EP_getEPIDAt,
			.fUpdateStatus	= (FTOM_CLOUD_CLIENT_EP_UPDATE_STATUS)FTOM_TP_CLIENT_EP_updateStatus,
			.fGetCount		= (FTOM_CLOUD_CLIENT_EP_GET_COUNT)FTOM_TP_CLIENT_EP_getCount,
			.fRegister		= (FTOM_CLOUD_CLIENT_EP_REGISTER)FTOM_TP_CLIENT_EP_register,
			.xData	=	
			{
				.fGetLastTime=(FTOM_CLOUD_CLIENT_EP_DATA_GET_LAST_TIME)FTOM_TP_CLIENT_EP_DATA_getLastTime,
				.fSend		= (FTOM_CLOUD_CLIENT_EP_DATA_SEND)FTOM_TP_CLIENT_EP_DATA_send
			}
		}
	}
};


FTM_RET	FTOM_CLOUD_CLIENT_getModule
(
	FTM_CHAR_PTR	pName,
	FTOM_CLOUD_CLIENT_MODULE_PTR _PTR_ ppModule
)
{
	ASSERT(pName != NULL);
	ASSERT(ppModule != NULL);
	FTM_INT	i;

	for(i = 0 ; i < sizeof(_pModules) / sizeof(FTOM_CLOUD_CLIENT_MODULE) ; i++)
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
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_destroy
(
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_init
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_final
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_start
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_stop
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_isRunning
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_waitingForFinished
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_load
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_save
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_CONFIG_show
(
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


FTM_RET	FTOM_DUMMY_CLIENT_setNotifyCB
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET FTOM_DUMMY_CLIENT_MESSAGE_send
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_DUMMY_CLIENT_MESSAGE_process
(
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
)
{
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


