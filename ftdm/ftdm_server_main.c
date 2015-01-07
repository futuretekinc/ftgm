#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "libconfig.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "debug.h"

#define	FTDM_FILE_PATH_LEN				256
#define	FTDM_FILE_NAME_LEN				256
#define	FTDM_DEFAULT_SERVICE_PORT		8888
#define	FTDM_PACKET_LEN					2048

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
	}	xNetwork;

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

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

static FTM_RET			FTDMS_init(FTDM_SERVER_CONFIG_PTR pServerConfig);
static FTM_VOID_PTR 	FTDMS_serviceHandler(FTM_VOID_PTR pData);
static FTM_RET 			FTDMS_startDaemon(FTDM_SERVER_CONFIG_PTR pServerConfig);
static FTM_VOID			FTDMS_showUsage(FTM_CHAR_PTR pAppName);

static FTDM_SERVER_CONFIG	xServerConfig =
{
	.xApp =
	{
		.pFileName = "",
	},
	.xDatabase = 
	{
		.pFileName = "",
	},
	.xNetwork = 
	{
		.usPort = FTDM_DEFAULT_SERVICE_PORT
	},
	.xDebug =
	{
		.ulPrintOutMode = 1,
		.xTrace = 
		{
			.pPath = "/var/log/",
			.pPrefix="ftdm_server"
		},
		.xError =
		{
			.pPath = "/var/log/",
			.pPrefix="ftdm_server"
		}
	}
};

int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_BOOL_FALSE;
	extern char *program_invocation_short_name;

	/* set default configuration */
	sprintf(xServerConfig.xApp.pFileName, "%s.conf", program_invocation_short_name);
	sprintf(xServerConfig.xDatabase.pFileName, "%s.db", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:dm:?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strncpy(xServerConfig.xApp.pFileName, optarg, FTDM_FILE_NAME_LEN);
			}
			break;

		case	'd':
			{
				bDaemon = FTM_BOOL_TRUE;
			}
			break;

		case	'm':
			{
				FTM_ULONG	ulPrintOutMode;

				ulPrintOutMode = strtoul(optarg, NULL, 10);
				if (ulPrintOutMode < 3)
				{
					xServerConfig.xDebug.ulPrintOutMode = ulPrintOutMode;	
				}
			}
			break;

		case	'?':
		default:
			FTDMS_showUsage(pArgv[0]);
			return	0;
		}
	}

	/* apply configuration */
	FTDMS_init(&xServerConfig);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTDMS_startDaemon(&xServerConfig);
		}
	}
	else
	{
		FTDMS_startDaemon(&xServerConfig);
	}



	return	0;
}

FTM_RET	FTDMS_init(FTDM_SERVER_CONFIG_PTR pConfig)
{
	config_t			xConfig;
	config_setting_t	*pSection;

	if (pConfig == NULL)
	{
		ERROR("Server configuration is NULL!\n");
		return	FTM_RET_INTERNAL_ERROR;	
	}

	setPrintMode(pConfig->xDebug.ulPrintOutMode);
	FTM_initEPTypeString();

	config_init(&xConfig);

	if (CONFIG_TRUE != config_read_file(&xConfig, pConfig->xApp.pFileName))
	{
		ERROR("Configuration loading failed.[FILE = %s]\n", pConfig->xApp.pFileName);
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "application");
	if (pSection)
	{
		config_setting_t *pDBSetting;
		config_setting_t *pNetworkSetting;

		pDBSetting = config_setting_get_member(pSection, "database");
		if (pDBSetting)
		{
			config_setting_t *pDBFileSetting;

			pDBFileSetting = config_setting_get_member(pDBSetting, "file");
			if (pDBFileSetting)
			{
				strcpy(pConfig->xDatabase.pFileName, 
					config_setting_get_string(pDBFileSetting));	
			}
		}

		pNetworkSetting = config_setting_get_member(pSection, "network");
		if (pNetworkSetting)
		{
			config_setting_t *pPortSetting;

			pPortSetting = config_setting_get_member(pNetworkSetting, "port");
			if (pPortSetting)
			{
				pConfig->xNetwork.usPort = config_setting_get_int(pPortSetting);
			}
		}
	}

	pSection = config_lookup(&xConfig, "endpoint");
	if (pSection)
	{
		FTM_INT	i;
		config_setting_t	*pTypeStringSetting;

		pTypeStringSetting = config_setting_get_member(pSection, "type_string");
		for( i = 0 ; i < config_setting_length(pTypeStringSetting) ; i++)
		{
			config_setting_t	*pElement;

			pElement = config_setting_get_elem(pTypeStringSetting, i);
			if (pElement != NULL)
			{
				FTM_INT		 nType = config_setting_get_int_elem(pElement, 0);	
				FTM_CHAR_PTR pTypeString = (FTM_CHAR_PTR)config_setting_get_string_elem(pElement, 1);	

				if (pTypeString != NULL)
				{
					FTM_appendEPTypeString(nType, pTypeString);	
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTDMS_startDaemon(FTDM_SERVER_CONFIG_PTR pConfig)
{
	int					nRet;
	int					hSocket;
	struct sockaddr_in	xServer, xClient;

	FTDM_init(pConfig->xDatabase.pFileName);

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	FTM_RET_ERROR;
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pConfig->xNetwork.usPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bind failed.\n");
		return	FTM_RET_ERROR;
	}

	listen(hSocket, 3);


	while(1)
	{
		int hClient;
		int	nSockAddrInLen = sizeof(struct sockaddr_in);	

		MESSAGE("Waiting for incoming connections ...\n");
		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	
			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)malloc(sizeof(FTDM_SESSION));
			if (pSession == NULL)
			{
				ERROR("System memory is not enough!\n");
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				TRACE("The new session has beed connected\n"\
					  "HANDLE : %08lx\n"\
					  "  PEER : %s:%d\n", 
					  hClient, 
					  inet_ntoa(xClient.sin_addr), 
					  ntohs(xClient.sin_port));

				pSession->hSocket = hClient;
				memcpy(&pSession->xPeer, &xClient, sizeof(xClient));
				pthread_create(&xPthread, NULL, FTDMS_serviceHandler, pSession);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_serviceHandler(FTM_VOID_PTR pData)
{
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;

	while(1)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, nLen);
		if (nLen == 0)
		{
			TRACE("The connection is terminated.\n");
			break;	
		}
		else if (nLen < 0)
		{
			ERROR("recv failed[%d]\n", -nLen);
			break;	
		}

		if (FTM_RET_OK != FTDMS_service(pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTM_RET_INTERNAL_ERROR;
			pResp->nLen = sizeof(FTDM_RESP_PARAMS);
		}

		TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->nLen);
		nLen = send(pSession->hSocket, pResp, pResp->nLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR("send failed[%d]\n", -nLen);	
			break;
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	return	0;
}

FTM_VOID	FTDMS_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-d] [-m 0|1|2]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for M2M gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -d      Run as a daemon\n");
	MESSAGE("    -m <n>  Set message output mode.\n");
	MESSAGE("            0 - Don't output any messages.\n");
	MESSAGE("            1 - Only general message\n");
	MESSAGE("            2 - All message(include debugging message).\n");
}
