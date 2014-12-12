#ifndef	__LIB_FTDM_H__
#define	__LIB_FTDM_H__

int	FTDM_init(void);
int	FTDM_final(void);

int	FTDM_devInsert(FTGM_DID xDID, FTGM_DEVICE_TYPE xType, char *strURL, char *strLocation);
int FTDM_devRemove(FTGM_DID xDID);

int	FTDM_devInfo(	
		FTGM_DID 				xDID, 
		FTGM_DEVICE_TYPE_PTR 	pType, 
		char 					*strURL, 
		int 					nURLLen, 
		char 					*strLoc, 
		int 					nLocLen);
int	FTDM_devInfoType(FTGM_DID xDID, FTGM_DEVICE_TYPE_PTR pType);

int FTDM_devInfoURL(FTGM_DID xDID, char *strBuff, int nBuffLen);
int FTDM_devInfoURLSet(FTGM_DID xDID, char *strBuff);

int FTDM_devInfoLocation(FTGM_DID xDID, char *strBuff, int nBuffLen);
int FTDM_devInfoLocationSet(FTGM_DID xDID, char *strBuff);

int	FTDM_epInsert(
		FTGM_EPID 				xEPID, 
		FTGM_DID 				xDID, 
		FTGM_EP_TYPE 			xType, 
		char 					*strName);
#endif

