#include <stdio.h>
#include <curl/curl.h>
#include "ftom.h"
#include "ftom_tp_client.h"

typedef	enum
{
	FTOM_TP_CLIENT_CMD_UNKNOWN = 0,
	FTOM_TP_CLIENT_CMD_GW_GET_INFO,
	FTOM_TP_CLIENT_CMD_GW_GET_STATUS,
	FTOM_TP_CLIENT_CMD_GW_SET_STATUS,
	FTOM_TP_CLIENT_CMD_GW_GET_MODEL,
	FTOM_TP_CLIENT_CMD_DEVICE_CREATE,
	FTOM_TP_CLIENT_CMD_DEVICE_DELETE,
	FTOM_TP_CLIENT_CMD_SENSOR_LIST,
	FTOM_TP_CLIENT_CMD_SENSOR_CREATE,
	FTOM_TP_CLIENT_CMD_SENSOR_DELETE,
	FTOM_TP_CLIENT_CMD_SENSOR_GET_STATUS,
	FTOM_TP_CLIENT_CMD_SENSOR_SET_STATUS,
	FTOM_TP_CLIENT_CMD_SENSOR_SET_VALUE
}	FTOM_TP_CLIENT_CMD, _PTR_ FTOM_TP_CLIENT_CMD_PTR;

typedef	struct
{
	FTOM_TP_CLIENT_CMD	xCmd;
	FTM_CHAR_PTR		pString;
}	FTOM_TP_CLIENT_CMD_INFO, _PTR_ FTOM_TP_CLIENT_CMD_INFO_PTR;

FTOM_TP_CLIENT_CMD_INFO	pTPClientCmds[] =
{
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_GW_GET_INFO,
		.pString= "gw_get_info"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_GW_GET_STATUS,
		.pString= "gw_get_status"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_GW_SET_STATUS,
		.pString= "gw_set_status"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_GW_GET_MODEL,
		.pString= "gw_get_model"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_DEVICE_CREATE,
		.pString= "device_create"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_DEVICE_DELETE,
		.pString= "device_delete"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_SENSOR_LIST,
		.pString= "sensor_list"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_SENSOR_CREATE,
		.pString= "sensor_create"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_SENSOR_DELETE,
		.pString= "sensor_delete"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_SENSOR_GET_STATUS,
		.pString= "sensor_get_status"
	},
	{
		.xCmd 	= FTOM_TP_CLIENT_CMD_SENSOR_SET_STATUS,
		.pString= "sensor_set_status"
	}
};

FTM_EP	xEP =
{
	.pEPID = "00405cabcdef-1232",
	.pName = "00405cabcdef-1232",
	.xType = FTM_EP_TYPE_TEMPERATURE
};

FTM_EP_DATA	xData =
{
	.ulTime = 0,
	.xType  = FTM_EP_DATA_TYPE_FLOAT,
	.xState = FTM_EP_DATA_STATE_VALID,
	.xValue = 
	{
		.xType = FTM_VALUE_TYPE_FLOAT
	}
};

FTM_BOOL	bVerbose = FTM_TRUE;
FTM_BOOL	bDataDump = FTM_TRUE;
FTM_BOOL	bStatus = FTM_FALSE;
FTM_ULONG	ulTimeout = 90;


int main(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET			xRet;
	FTOM_TP_CLIENT	xClient;
	FTM_INT			nOpt;
	FTM_CHAR		pGatewayID[FTM_DID_LEN+1] = {0,};
	FTM_CHAR		pDeviceID[FTM_DID_LEN+1] = {0,};
	FTM_CHAR		pSensorID[FTM_EPID_LEN+1] = {0,};
	FTM_CHAR		pSensorType[FTM_NAME_LEN+1] = {0,};
	FTOM_TP_CLIENT_CMD		xCmd = FTOM_TP_CLIENT_CMD_UNKNOWN;

	while((nOpt = getopt(nArgc, pArgv, "c:g:s:t:h?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				FTM_INT	i;

				for(i = 0 ; i < sizeof(pTPClientCmds) /sizeof(FTOM_TP_CLIENT_CMD_INFO) ; i++)
				{
					if (strcasecmp(optarg, pTPClientCmds[i].pString) == 0)
					{
						xCmd = pTPClientCmds[i].xCmd;
						break;
					}
				}

			}
			break;
		
		case	'g':
			{
				strncpy(pGatewayID, optarg, FTM_DID_LEN);
			}
			break;

		case	'd':
			{
				strncpy(pDeviceID, optarg, FTM_DID_LEN);
			}
			break;

		case	's':
			{
				strncpy(pSensorID, optarg, FTM_EPID_LEN);
			}
			break;

		case	't':
			{
				strncpy(pSensorType, optarg, FTM_NAME_LEN);
			}
			break;

		default:
			return	0;
		}
	}

	FTM_MEM_init();
	FTM_TRACE_setLevel(0);

	xRet = FTOM_TP_CLIENT_init(&xClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Thingplus client initialization failed.\n");
		return	0;	
	}

	FTOM_TP_CLIENT_setVerbose(bVerbose);
	FTOM_TP_CLIENT_setDataDump(bDataDump);

	xRet = FTOM_TP_CLIENT_GW_setID(&xClient, pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to set gateway ID!\n");	
		ERROR("Gateway ID : %s\n", pGatewayID);	
	}

	switch(xCmd)
	{
	case	FTOM_TP_CLIENT_CMD_GW_GET_INFO:
		{
			xRet = FTOM_TP_CLIENT_GW_getInfo(&xClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to get gateway information.\n");
			}
		}
		break;
#if 0
	case	FTOM_TP_CLIENT_CMD_GW_GET_STATUS:
		{
			xRet = FTOM_TP_CLIENT_GW_getStatus(&xClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to get gateway status.\n");
			}
		}
		break;
#endif
	case	FTOM_TP_CLIENT_CMD_GW_SET_STATUS:
		{
			if (optind >= nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			if (strcasecmp(pArgv[optind], "on") == 0)
			{
				optind++;
				bStatus = FTM_TRUE;	
			}
			else if (strcasecmp(pArgv[optind], "off") == 0)
			{
				optind++;
				bStatus = FTM_FALSE;	
			}
			else
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			if (optind <  nArgc)
			{
				ulTimeout = strtoul(pArgv[optind], 0, 10);
			}

			xRet = FTOM_TP_CLIENT_GW_setStatus(&xClient, bStatus, ulTimeout);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to set gateway status.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_GW_GET_MODEL:
		{
			FTM_ULONG	ulModelID ;

			if (optind >= nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}
		
			ulModelID = strtoul(pArgv[optind++], 0, 10);

			xRet = FTOM_TP_CLIENT_GW_getModel(&xClient, ulModelID);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to set gateway status.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_DEVICE_CREATE:
		{
			if ((optind + 2) != nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			xRet = FTOM_TP_CLIENT_DEVICE_create(&xClient, 
						pDeviceID, 
						pArgv[optind], 
						pArgv[optind+1]);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to create device.\n");	
			}
		}
		break;
	case	FTOM_TP_CLIENT_CMD_SENSOR_LIST:
		{
			xRet = FTOM_TP_CLIENT_SENSOR_getList(&xClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to get gateway information.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_SENSOR_CREATE:
		{
			if ((optind + 1) != nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			xRet = FTOM_TP_CLIENT_SENSOR_create(&xClient, 
						pGatewayID, 
						pSensorID, 
						pSensorType,
						pArgv[optind],
						NULL,
						"0",
						"0");
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to create Sensor.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_SENSOR_DELETE:
		{
			xRet = FTOM_TP_CLIENT_SENSOR_delete(&xClient, pSensorID);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to sensor delete.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_SENSOR_SET_STATUS:
		{
			if (optind >= nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			if (strcasecmp(pArgv[optind], "on") == 0)
			{
				optind++;
				bStatus = FTM_TRUE;	
			}
			else if (strcasecmp(pArgv[optind], "off") == 0)
			{
				optind++;
				bStatus = FTM_FALSE;	
			}
			else
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			if (optind <  nArgc)
			{
				ulTimeout = strtoul(pArgv[optind], 0, 10);
			}

			xRet = FTOM_TP_CLIENT_SENSOR_setStatus(&xClient, pSensorID, bStatus, ulTimeout);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to sensor delete.\n");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_SENSOR_GET_STATUS:
		{
			FTM_BOOL	bStatus = FTM_FALSE;

			xRet = FTOM_TP_CLIENT_SENSOR_getStatus(&xClient, pSensorID, &bStatus);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to sensor delete.\n");
			}
			else
			{
				MESSAGE("Sensor[%s] is %s.\n",	pSensorID, (bStatus)?"on":"off");
			}
		}
		break;

	case	FTOM_TP_CLIENT_CMD_SENSOR_SET_VALUE:
		{
			FTM_FLOAT	fValue;
			FTM_EP_DATA	xData;

			if (optind >= nArgc)
			{
				ERROR("Invalid arguments!\n");	
				break;
			}

			fValue = strtod(pArgv[optind++], NULL);

			FTM_EP_DATA_initFLOAT(&xData, fValue);
			
			xRet = FTOM_TP_CLIENT_SENSOR_setValues(&xClient, pSensorID, &xData, 1);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to set sensor data.\n");
			}
		}
		break;


	default:
		ERROR("Invalid command!\n");	
		break;
	}

	FTOM_TP_CLIENT_final(&xClient);
	FTM_MEM_final();

  	return 0;
}
