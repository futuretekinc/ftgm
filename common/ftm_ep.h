#ifndef	__FTM_EP_H__
#define __FTM_EP_H__

#include "ftm_node.h"
#include "ftm_value.h"

typedef	unsigned long	FTM_EP_TYPE, 	_PTR_ FTM_EP_TYPE_PTR;
typedef	unsigned long	FTM_EP_STATE, 	_PTR_ FTM_EP_STATE_PTR;

#define	FTM_EP_STATE_DISABLE		0x00000000
#define	FTM_EP_STATE_RUN			0x00000001
#define	FTM_EP_STATE_STOP			0x00000002
#define	FTM_EP_STATE_ERROR			0x00000003

#define	FTM_EP_TYPE_MASK			0x7F000000
#define	FTM_EP_TYPE_TEMPERATURE		0x01000000
#define	FTM_EP_TYPE_HUMIDITY		0x02000000
#define	FTM_EP_TYPE_VOLTAGE			0x03000000
#define	FTM_EP_TYPE_CURRENT			0x04000000
#define	FTM_EP_TYPE_DI				0x05000000
#define	FTM_EP_TYPE_DO				0x06000000
#define	FTM_EP_TYPE_GAS				0x07000000
#define	FTM_EP_TYPE_POWER			0x08000000
#define	FTM_EP_TYPE_AI				0x0A000000
#define	FTM_EP_TYPE_COUNT			0x0B000000
#define	FTM_EP_TYPE_PRESSURE		0x0C000000
#define	FTM_EP_TYPE_DISCRETE		0x0D000000

#define	FTM_EP_TYPE_DEVICE			0x70000000
#define	FTM_EP_TYPE_MULTI			0x7E000000
#define	FTM_EP_TYPE_CTRL			0x7F000000

typedef	unsigned long	FTM_EP_FIELD, _PTR_ FTM_EP_FIELD_PTR;

#define	FTM_EP_FIELD_ALL			(0xFFFFFFFF)
#define	FTM_EP_FIELD_EPID			(1 << 0)
#define	FTM_EP_FIELD_EPTYPE			(1 << 1)
#define	FTM_EP_FIELD_FLAGS			(1 << 2)
#define	FTM_EP_FIELD_NAME			(1 << 3)
#define	FTM_EP_FIELD_UNIT			(1 << 4)
#define	FTM_EP_FIELD_ENABLE			(1 << 5)
#define	FTM_EP_FIELD_TIMEOUT		(1 << 6)
#define	FTM_EP_FIELD_INTERVAL		(1 << 7)
#define	FTM_EP_FIELD_REPORT_INTERVAL (1 << 8)
#define	FTM_EP_FIELD_DID			(1 << 9)
#define	FTM_EP_FIELD_LIMIT			(1 << 10)

typedef enum
{
	FTM_EP_FLAG_STATIC = (1 << 0),
	FTM_EP_FLAG_SYNC   = (1 << 1)
}	FTM_EP_FLAG, _PTR_ FTM_EP_FLAG_PTR;

typedef	enum
{
	FTM_EP_DATA_STATE_VALID =0,
	FTM_EP_DATA_STATE_INVALID,
} FTM_EP_DATA_STATE, _PTR_ FTM_EP_DATA_STATE_PTR;

typedef	struct FTM_EP_DATA_STRUCT
{
	FTM_ULONG			ulTime;
	FTM_EP_DATA_STATE	xState;
	FTM_VALUE			xValue;
}	FTM_EP_DATA, _PTR_ FTM_EP_DATA_PTR;

typedef	enum
{
	FTM_EP_LIMIT_TYPE_COUNT = 0,
	FTM_EP_LIMIT_TYPE_TIME,
	FTM_EP_LIMIT_TYPE_HOURS,
	FTM_EP_LIMIT_TYPE_DAYS,
	FTM_EP_LIMIT_TYPE_MONTHS
}	FTM_EP_LIMIT_TYPE, _PTR_ FTM_EP_LIMIT_TYPE_PTR;

typedef	struct
{
	FTM_EP_TYPE		xType;	
	FTM_CHAR		pID[32];
	FTM_CHAR		pName[32];
	FTM_CHAR		pSN[32];
	FTM_CHAR		pState[32];
	FTM_CHAR		pValue[32];
	FTM_CHAR		pTime[32];
}	FTM_EP_CLASS, _PTR_ FTM_EP_CLASS_PTR;

typedef	struct
{
	FTM_ULONG		ulStart;
	FTM_ULONG		ulEnd;
}	FTM_EP_LIMIT_TIME, _PTR_ FTM_EP_LIMIT_TIME_PTR;

typedef	struct
{
	FTM_EP_LIMIT_TYPE	xType;
	struct
	{
		FTM_ULONG			ulCount;
		FTM_EP_LIMIT_TIME	xTime;
		FTM_ULONG			ulHours;
		FTM_ULONG			ulDays;
		FTM_ULONG			ulMonths;
	}	xParams;
}	FTM_EP_LIMIT, _PTR_ FTM_EP_LIMIT_PTR;

typedef	enum
{
	FTM_EP_CTRL_OFF,
	FTM_EP_CTRL_ON,
	FTM_EP_CTRL_BLINK
}	FTM_EP_CTRL, _PTR_ FTM_EP_CTRL_PTR;
	
typedef	struct
{
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_TYPE		xType;
	FTM_EP_FLAG		xFlags;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pUnit[FTM_UNIT_LEN+1];
	FTM_BOOL		bEnable;
	FTM_ULONG		ulTimeout;
	FTM_ULONG		ulUpdateInterval;	
	FTM_ULONG		ulReportInterval;
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_ULONG		xDEPID;
	FTM_EP_LIMIT	xLimit;
}	FTM_EP, _PTR_ FTM_EP_PTR;

FTM_RET	FTM_EP_init
(
	FTM_VOID
);

FTM_RET	FTM_EP_final
(
	FTM_VOID
);

FTM_RET	FTM_EP_setDefault
(
	FTM_EP_PTR 	pEP
);

FTM_RET	FTM_EP_create
(
	FTM_EP_PTR 	pSrc, 
	FTM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTM_EP_destroy
(
	FTM_EP_PTR 	pNode
);

FTM_RET	FTM_EP_append
(
	FTM_EP_PTR 	pEP
);

FTM_RET	FTM_EP_remove
(
	FTM_EP_PTR 	pEP
);

FTM_RET	FTM_EP_count
(
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_EP_get
(
	FTM_CHAR_PTR 	pEPID, 
	FTM_EP_PTR _PTR_ ppNode
);

FTM_RET	FTM_EP_getAt
(
	FTM_ULONG 	ulIndex, 
	FTM_EP_PTR _PTR_ ppNode
);

FTM_RET	FTM_EP_setFields
(
	FTM_EP_PTR		pEP,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTM_EP_isValid
(
	FTM_EP_PTR 	pEP
);

FTM_RET	FTM_EP_isValidTimeout
(
	FTM_EP_PTR 	pEP, 
	FTM_ULONG 	ulTimeout
);

FTM_RET	FTM_EP_isStatic
(
	FTM_EP_PTR 	pEP
);

FTM_RET	FTM_EP_isAsyncMode
(
	FTM_EP_PTR 	pEP
);

FTM_CHAR_PTR FTM_EP_typeString
(
	FTM_EP_TYPE nType
);

FTM_RET	FTM_EP_strToType
(
	FTM_CHAR_PTR	pString,
	FTM_EP_TYPE_PTR	pType
);

FTM_RET	FTM_EP_getDataType
(
	FTM_EP_PTR 	pEP, 
	FTM_VALUE_TYPE_PTR pType
);

FTM_RET			FTM_initEPTypeString
(
	FTM_VOID
);

FTM_RET			FTM_finalEPTypeString
(
	FTM_VOID
);

FTM_RET			FTM_appendEPTypeString
(
	FTM_EP_TYPE xType, 
	FTM_CHAR_PTR pTypeString
);

FTM_CHAR_PTR 	FTM_getEPTypeString
(
	FTM_EP_TYPE nType
);

FTM_RET	FTM_EP_getDefaultUnit
(
	FTM_EP_TYPE		nType,
	FTM_CHAR_PTR	pUnit,
	FTM_ULONG		ulLen
);

FTM_RET	FTM_EP_DATA_create
(
	FTM_EP_DATA_PTR pSrcData, 
	FTM_EP_DATA_PTR _PTR_ ppData
);

FTM_RET	FTM_EP_DATA_createInt
(
	FTM_INT nValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
);

FTM_RET	FTM_EP_DATA_createUlong
(
	FTM_ULONG ulValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
);

FTM_RET	FTM_EP_DATA_createFloat
(
	FTM_DOUBLE fValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
);

FTM_RET	FTM_EP_DATA_createBool
(	
	FTM_BOOL bValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
);

FTM_RET	FTM_EP_DATA_init
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_TYPE	xType,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_EP_DATA_initINT
(
	FTM_EP_DATA_PTR pData,
	FTM_INT 		nValue 
);

FTM_RET	FTM_EP_DATA_initULONG
(
	FTM_EP_DATA_PTR pData,
	FTM_ULONG 		ulValue 
);

FTM_RET	FTM_EP_DATA_initFLOAT
(
	FTM_EP_DATA_PTR pData,
	FTM_DOUBLE 		fValue
);

FTM_RET	FTM_EP_DATA_initBOOL
(	
	FTM_EP_DATA_PTR pData,
	FTM_BOOL 		bValue 
);

FTM_RET	FTM_EP_DATA_initVALUE
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_PTR	pValue
);

FTM_RET	FTM_EP_DATA_initValueFromString
(
	FTM_EP_DATA_PTR		pData,
	FTM_VALUE_TYPE		xType,
	FTM_CHAR_PTR		pValue
);


FTM_RET	FTM_EP_DATA_final
(
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTM_EP_DATA_setValueFromString
(
	FTM_EP_DATA_PTR	pData,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_EP_DATA_destroy
(
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTM_EP_DATA_compare
(
	FTM_EP_DATA_PTR pData1, 
	FTM_EP_DATA_PTR pData2, 
	FTM_INT_PTR pResult
);

FTM_RET	FTM_EP_DATA_toValue
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_PTR	pValue
);

FTM_RET	FTM_EP_print
(
	FTM_EP_PTR	pEP
);

typedef	struct
{
	FTM_USHORT			usPort;
	FTM_ULONG			ulSessionCount;
}	FTM_SERVER_INFO, _PTR_ FTM_SERVER_INFO_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pFileName;
}	FTM_DB_INFO, _PTR_ FTM_DB_INFO_PTR;

FTM_RET	FTM_EP_DATA_snprint
(
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulMaxLen, 
	FTM_EP_DATA_PTR pData
);

FTM_CHAR_PTR	FTM_EP_DATA_print
(
	FTM_EP_DATA_PTR pData
);
#endif

