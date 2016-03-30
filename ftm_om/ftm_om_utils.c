#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>   
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>  
#include <unistd.h> 
#include "ftm.h"

FTM_RET	FTM_OM_getMACAddress(FTM_CHAR_PTR pIFName, FTM_UINT8	pMAC[6]);

static	FTM_CHAR_PTR	pIFList[] =
{
	"eth0",
	"eth1",
	"eth2"
};

FTM_RET	FTM_OM_getDefaultDeviceID(FTM_CHAR	pDID[FTM_DID_LEN + 1])
{
	FTM_INT		i;
	FTM_RET		xRet;
	FTM_UINT8	pMAC[6];

	for(i = 0 ; i < sizeof(pIFList) / sizeof(pIFList[0]) ; i++)
	{
		xRet = FTM_OM_getMACAddress(pIFList[i], pMAC);	
		if (xRet == FTM_RET_OK)
		{
			sprintf(pDID, "%02X%02X%02X%02X%02X%02X", 
				pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
			return	FTM_RET_OK;
		}
	}

	sprintf(pDID, "%08X%08X%08X", rand(), rand(), rand());

	return FTM_RET_OK;
}

FTM_RET	FTM_OM_getMACAddress(FTM_CHAR_PTR pIFName, FTM_UINT8 pMAC[6])
{
	FTM_INT			nFD;
	FTM_INT			nRet;
	struct ifreq 	xIFReq;

	memset(&xIFReq, 0, sizeof(xIFReq));

	nFD = socket(AF_INET, SOCK_DGRAM, 0);

	xIFReq.ifr_addr.sa_family = AF_INET;
	strncpy(xIFReq.ifr_name , pIFName, IFNAMSIZ-1);

	nRet = ioctl(nFD, SIOCGIFHWADDR, &xIFReq); 
	close(nFD);

	if (nRet == 0)
	{
		memcpy(pMAC, (FTM_UINT8_PTR)xIFReq.ifr_hwaddr.sa_data, 6);
		return	FTM_RET_OK;
	}

	return	FTM_RET_NOT_EXISTS;
}
