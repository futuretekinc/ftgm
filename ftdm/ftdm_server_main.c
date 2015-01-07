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


#define	FTDM_DEFAULT_SERVICE_PORT		8888
#define	FTDM_PACKET_LEN					2048

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

static FTM_RET			FTDMS_init(FTM_CHAR_PTR pConfigFile);
static FTM_VOID_PTR 	FTDMS_serviceHandler(FTM_VOID_PTR pData);
static FTM_RET 			FTDMS_startDaemon(void);

static	FTM_USHORT		usServicePort = FTDM_DEFAULT_SERVICE_PORT;
static	FTM_CHAR		pConfigFileName[1024];
static	FTM_CHAR		pDBFileName[1024];

int main(int nArgc, char *pArgv[])
{
	FTM_INT	nOpt;
	pid_t	pid;

	sprintf(pConfigFileName, "%s.conf", pArgv[0]);

	while((nOpt = getopt(nArgc, pArgv, "P:SV")) != -1)
	{
		switch(nOpt)
		{
		case	'P':
			usServicePort = strtoul(optarg, NULL, 10);
			break;

		case	'S':
			setPrintMode(0);
			break;

		case	'V':
			setPrintMode(2);
			break;

		default:
			break;
		}
	}

	FTDMS_init(pConfigFileName);

	pid = fork();
	if (pid == 0)
	{
		FTDMS_startDaemon();
	}


	return	0;
}

FTM_RET	FTDMS_init(FTM_CHAR_PTR pConfigFile)
{
	FTM_initEPTypeString();

	if (pConfigFile != NULL)
	{
		config_t			xConfig;
		config_setting_t	*pSection;

		config_init(&xConfig);

		if (CONFIG_TRUE != config_read_file(&xConfig, pConfigFile))
		{
			ERROR("Configuration loading failed.[FILE = %s]\n", pConfigFile);
			return	FTM_RET_CONFIG_LOAD_FAILED;
		}

		pSection = config_lookup(&xConfig, "type_string");
		if (pSection)
		{
			FTM_INT	i;

			for( i = 0 ; i < config_setting_length(pSection) ; i++)
			{
				config_setting_t	*pElement;

				pElement = config_setting_get_elem(pSection, i);
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

		pSection = config_lookup(&xConfig, "default");
		if (pSection)
		{
			config_setting_t *pServerSetting;
			config_setting_t *pDBSetting;

			pServerSetting = config_setting_get_member(pSection, "server");
			if (pServerSetting)
			{
				config_setting_t *pPortSetting;

				pPortSetting = config_setting_get_member(pServerSetting, "port");
				if (pPortSetting)
				{
					usServicePort = config_setting_get_int(pPortSetting);
				}
			}


			pDBSetting = config_setting_get_member(pSection, "database");
			if (pDBSetting)
			{
				config_setting_t *pDBFileSetting;

				pDBFileSetting = config_setting_get_member(pDBSetting, "file");
				if (pDBFileSetting)
				{
					strcpy(pDBFileName, config_setting_get_string(pDBFileSetting));	
				}
			}
		}
	}

	TRACE("CONFIGURATOIN\n");
	TRACE("%16s %lu\n","Port", usServicePort);
	return	FTM_RET_OK;
}

FTM_RET FTDMS_startDaemon(void)
{
	int					nRet;
	int					hSocket;
	struct sockaddr_in	xServer, xClient;

	FTDM_init(pDBFileName);

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	FTM_RET_ERROR;
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( usServicePort );

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

