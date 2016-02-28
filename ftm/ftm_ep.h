#ifndef	__FTM_EP_H__
#define __FTM_EP_H__

#include "ftm_node.h"

#define	FTM_EP_NAME_LEN						256
#define	FTM_EP_UNIT_LEN						32

typedef	unsigned long	FTM_EP_CLASS, _PTR_ FTM_EP_CLASS_PTR;
typedef	unsigned long	FTM_EP_ID, _PTR_ FTM_EP_ID_PTR;
typedef	unsigned long	FTM_EP_TYPE, _PTR_ FTM_EP_TYPE_PTR;

typedef	unsigned long	FTM_EP_STATE, _PTR_ FTM_EP_STATE_PTR;

#define	FTM_EP_STATE_DISABLE		0x00000000
#define	FTM_EP_STATE_RUN			0x00000001
#define	FTM_EP_STATE_STOP			0x00000002
#define	FTM_EP_STATE_ERROR			0x00000003

#define	FTM_EP_CLASS_MASK			0x7F000000
#define	FTM_EP_CLASS_TEMPERATURE	0x01000000
#define	FTM_EP_CLASS_HUMIDITY		0x02000000
#define	FTM_EP_CLASS_VOLTAGE		0x03000000
#define	FTM_EP_CLASS_CURRENT		0x04000000
#define	FTM_EP_CLASS_DI				0x05000000
#define	FTM_EP_CLASS_DO				0x06000000
#define	FTM_EP_CLASS_GAS			0x07000000
#define	FTM_EP_CLASS_POWER			0x08000000
#define	FTM_EP_CLASS_AI				0x0A000000
#define	FTM_EP_CLASS_COUNT			0x0B000000
#define	FTM_EP_CLASS_MULTI			0x7F000000

typedef	struct
{
	FTM_EP_CLASS	xClass;	
	struct
	{
		FTM_CHAR	pID[32];
		FTM_CHAR	pType[32];
		FTM_CHAR	pName[32];
		FTM_CHAR	pSN[32];
		FTM_CHAR	pState[32];
		FTM_CHAR	pValue[32];
		FTM_CHAR	pTime[32];
	}	xOIDs;
}	FTM_EP_CLASS_INFO, _PTR_ FTM_EP_CLASS_INFO_PTR;

#define	FTM_EP_TYPE_MASK			0x7FFF0000

typedef	struct
{
	FTM_EP_ID		xEPID;
	FTM_EP_TYPE		xType;
	FTM_CHAR		pName[FTM_EP_NAME_LEN+1];
	FTM_CHAR		pUnit[FTM_EP_UNIT_LEN+1];
	FTM_EP_STATE	xState;
	FTM_ULONG		ulTimeout;
	FTM_ULONG		ulInterval;	
	FTM_ULONG		ulPeriod;
	FTM_ULONG		ulLimit;
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_EP_ID		xDEPID;
	FTM_CHAR		pPID[FTM_DID_LEN+1];
	FTM_EP_ID		xPEPID;
}	FTM_EP, _PTR_ FTM_EP_PTR;

FTM_RET			FTM_initEPTypeString(void);
FTM_RET			FTM_finalEPTypeString(void);
FTM_RET			FTM_appendEPTypeString(FTM_EP_TYPE xType, FTM_CHAR_PTR pTypeString);
FTM_CHAR_PTR	FTM_getEPTypeString(FTM_EP_TYPE xType);

typedef	unsigned long	FTM_EP_DATA_TYPE, _PTR_ FTM_EP_DATA_TYPE_PTR;

#define	FTM_EP_DATA_TYPE_INT	0
#define	FTM_EP_DATA_TYPE_ULONG	1
#define	FTM_EP_DATA_TYPE_FLOAT	2
#define	FTM_EP_DATA_TYPE_BOOL	3

typedef	enum
{
	FTM_EP_DATA_STATE_VALID =0,
	FTM_EP_DATA_STATE_INVALID,
} FTM_EP_DATA_STATE, _PTR_ FTM_EP_DATA_STATE_PTR;

typedef	struct
{
	FTM_ULONG			ulTime;
	FTM_EP_DATA_TYPE	xType;
	FTM_EP_DATA_STATE	xState;
	union 
	{
		FTM_INT		nValue;
		FTM_ULONG	ulValue;
		FTM_DOUBLE	fValue;
		FTM_BOOL	bValue;
	}	xValue;
}	FTM_EP_DATA, _PTR_ FTM_EP_DATA_PTR;

FTM_RET	FTM_EP_DATA_createCopy(FTM_EP_DATA_PTR pSrcData, FTM_EP_DATA_PTR _PTR_ ppData);
FTM_RET	FTM_EP_DATA_createInt(FTM_INT nValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData);
FTM_RET	FTM_EP_DATA_createUlong(FTM_ULONG ulValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData);
FTM_RET	FTM_EP_DATA_createFloat(FTM_DOUBLE fValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData);
FTM_RET	FTM_EP_DATA_createBool(FTM_BOOL bValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData);

FTM_RET	FTM_EP_DATA_destroy(FTM_EP_DATA_PTR pData);

FTM_RET	FTM_EP_DATA_compare(FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_INT_PTR pResult);

FTM_CHAR_PTR FTM_nodeTypeString(FTM_NODE_TYPE nType);


typedef	struct
{
	FTM_USHORT			usPort;
	FTM_ULONG			ulSessionCount;
}	FTM_SERVER_INFO, _PTR_ FTM_SERVER_INFO_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pFileName;
}	FTM_DB_INFO, _PTR_ FTM_DB_INFO_PTR;

FTM_RET	FTM_EP_DATA_snprint(FTM_CHAR_PTR pBuff, FTM_ULONG ulMaxLen, FTM_EP_DATA_PTR pData);

#endif

