#include <signal.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32
#  include <unistd.h>
#else
#  include <process.h>
#  define snprintf sprintf_s
#endif

#include <mosquitto.h>

#include "ftm.h"
#include "ftm_mqtt_client.h"

static int run = 1;

FTM_MQTT_CLIENT_CONFIG	xMQTTCConfig =
{
	.pClientID="ftmqc",
	.xBroker =
	{
		.pHost = "127.0.0.1",
		.usPort= 1883
	}
};

FTNM_CLIENT_CONFIG	xFTNMCConfig = 
{
	.xServer =
	{
		.pHost		= "127.0.0.1",
		.usPort		= 8889
	}
};

void FTM_MQTT_CLIENT_signalHandle(int s)
{
	run = 0;
}

FTM_RET	FTNM_CLIENT_notifyCB(FTM_VOID_PTR pData)
{
	TRACE("Received Message!\n");
	return	FTM_RET_OK;
}

int main(int argc, char *argv[])
{
	FTM_MQTT_CLIENT_PTR	pMQTTC;
	FTNM_CLIENT		xFTNMC;

	FTM_RET	xRet;

	FTM_MEM_init();

//	signal(SIGINT, FTM_MQTT_CLIENT_signalHandle);
//	signal(SIGTERM, FTM_MQTT_CLIENT_signalHandle);

	xRet = FTM_MQTT_CLIENT_create(&pMQTTC);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client initialization fialed.[%08x]\n", xRet);
		return	0;
	}

	xRet = FTNM_CLIENT_init(&xFTNMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTNM Client initialization fialed.[%08x]\n", xRet);
		return	0;
	}

	FTM_MQTT_CLIENT_loadConfig(pMQTTC, &xMQTTCConfig);
	FTNM_CLIENT_loadConfig(&xFTNMC, &xFTNMCConfig);

	FTNM_CLIENT_setNotifyCallback(&xFTNMC, FTNM_CLIENT_notifyCB);

	FTNM_CLIENT_start(&xFTNMC);
	FTM_MQTT_CLIENT_start(pMQTTC);

	FTM_SHELL_init();
	FTM_SHELL_setPrompt("FTMQC> ");
	//FTM_SHELL_addCmds(FTDMS_pCmdList, FTDMS_ulCmdCount);
	FTM_SHELL_run();

	FTM_MQTT_CLIENT_stop(pMQTTC);
	FTNM_CLIENT_stop(&xFTNMC);

	FTM_MQTT_CLIENT_destroy(&pMQTTC);
	FTNM_CLIENT_final(&xFTNMC);

	FTM_MEM_init();

	return 0;
}

