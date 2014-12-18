#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

typedef struct 
{
	FTGM_INT	hSock;
	FTGM_INT	nTimeout;
}	FTDM_CLIENT, * FTDM_CLIENT_PTR;

FTGM_RET FTDMC_connect
(
 	FTGM_STRING 		strAddress, 
	FTGM_USHORT 		nPort, 
	FTDM_CLIENT_PTR 	pClient
);

FTGM_RET FTDMC_disconnect
(
	FTDM_CLIENT_PTR 	pClient
);

FTGM_RET FTDMC_request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTGM_BYTE_PTR 		pData, 
	FTGM_INT 			nDataLen, 
	FTGM_BYTE_PTR 		pBuff, 
	FTGM_INT_PTR 		pBuffLen
);

FTGM_RET FTDMC_devInfo
(
 	FTDM_CLIENT_PTR			pClient,
	FTGM_DEVICE_ID			xDID,
	FTGM_DEVICE_INFO_PTR	pInfo
);

#endif

