#ifndef	__FTM_VALUE_H__
#define	__FTM_VALUE_H__

#include "ftm_types.h"

typedef	FTM_ULONG	FTM_VALUE_TYPE;

#define	FTM_VALUE_DYNAMIC_MAGIC	0xA55AC33C

#define	FTM_VALUE_TYPE_BOOL		1
#define	FTM_VALUE_TYPE_INT		2
#define	FTM_VALUE_TYPE_USHORT	3
#define	FTM_VALUE_TYPE_ULONG	4
#define	FTM_VALUE_TYPE_FLOAT	5
#define	FTM_VALUE_TYPE_STRING	6

typedef	struct
{
	FTM_ULONG		xMagic;	
	FTM_VALUE_TYPE	xType;
	union
	{
		FTM_BOOL		bValue;
		FTM_INT			nValue;
		FTM_USHORT		usValue;
		FTM_ULONG		ulValue;	
		FTM_FLOAT		fValue;	
		FTM_CHAR_PTR	pValue;	
	}	xValue;
	FTM_ULONG		ulLen;
}	FTM_VALUE, _PTR_ FTM_VALUE_PTR;

FTM_RET	FTM_VALUE_create
(
	FTM_VALUE_PTR _PTR_ ppObjevt, 
	FTM_VALUE_TYPE xType
);

FTM_RET	FTM_VALUE_destroy
(
	FTM_VALUE_PTR pObject
);

FTM_RET	FTM_VALUE_init
(
	FTM_VALUE_PTR 	pObject, 
	FTM_VALUE_TYPE 	xType,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_VALUE_initBOOL
(
	FTM_VALUE_PTR 	pObject, 
	FTM_BOOL		bValue
);

FTM_RET	FTM_VALUE_initINT
(
	FTM_VALUE_PTR 	pObject, 
	FTM_INT			nValue
);

FTM_RET	FTM_VALUE_initULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG		ulValue
);

FTM_RET	FTM_VALUE_initFLOAT
(
	FTM_VALUE_PTR 	pObject, 
	FTM_FLOAT		fValue
);

FTM_RET	FTM_VALUE_initString
(
	FTM_VALUE_PTR 	pObject, 
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_VALUE_final
(
	FTM_VALUE_PTR 	pValue
);

FTM_BOOL	FTM_VALUE_isBOOL
(
	FTM_VALUE_PTR 	pObject
);

FTM_BOOL	FTM_VALUE_isINT
(
	FTM_VALUE_PTR pObject
);

FTM_BOOL	FTM_VALUE_isFLOAT
(
	FTM_VALUE_PTR pObject
);

FTM_BOOL	FTM_VALUE_isUSHORT
(
	FTM_VALUE_PTR pObject
);

FTM_BOOL	FTM_VALUE_isULONG
(
	FTM_VALUE_PTR pObject
);

FTM_BOOL	FTM_VALUE_isSTRING
(
	FTM_VALUE_PTR pObject
);

FTM_RET	FTM_VALUE_setINT
(
	FTM_VALUE_PTR	pObject,
	FTM_INT			nValue
);

FTM_RET	FTM_VALUE_setULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG 		ulValue
);

FTM_RET	FTM_VALUE_getULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG_PTR 	pulValue
);

FTM_RET	FTM_VALUE_setFLOAT
(
	FTM_VALUE_PTR	pObject,
	FTM_FLOAT		fValue
);

FTM_RET	FTM_VALUE_setBOOL
(
	FTM_VALUE_PTR	pObject,
	FTM_BOOL		bValue
);

FTM_RET	FTM_VALUE_compare
(
	FTM_VALUE_PTR pValue1, 
	FTM_VALUE_PTR pValue2, 
	FTM_INT_PTR pResult
);

FTM_RET	FTM_VALUE_snprint
(	
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulMaxLen, 
	FTM_VALUE_PTR 	pObject
);

FTM_CHAR_PTR	FTM_VALUE_print
(	
	FTM_VALUE_PTR 	pObject
);

#endif

