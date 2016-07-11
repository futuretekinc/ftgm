#ifndef __FTM_JSON_H__
#define __FTM_JSON_H__

#define FTM_JSON_RET_OK     0
#define FTM_JSON_RET_ERROR  1
typedef enum
{
    FTM_JSON_TYPE_STRING = 0,
    FTM_JSON_TYPE_NUMBER,
    FTM_JSON_TYPE_FLOAT,
    FTM_JSON_TYPE_PAIR,   
    FTM_JSON_TYPE_OBJECT,   
    FTM_JSON_TYPE_ARRAY,
    FTM_JSON_TYPE_TRUE,
    FTM_JSON_TYPE_FALSE,
    FTM_JSON_TYPE_HEX,
    FTM_JSON_TYPE_NULL
}   FTM_JSON_TYPE, _PTR_ FTM_JSON_TYPE_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
}   FTM_JSON_VALUE, * FTM_JSON_VALUE_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_CHAR_PTR		pValue;
}   FTM_JSON_STRING, * FTM_JSON_STRING_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_INT				nValue;
}   FTM_JSON_NUMBER, * FTM_JSON_NUMBER_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_INT				nValue;
}   FTM_JSON_HEX, * FTM_JSON_HEX_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_FLOAT			fValue;
}   FTM_JSON_FLOAT, * FTM_JSON_FLOAT_PTR;

typedef struct _FTM_JSON_PAIR
{
    FTM_JSON_TYPE       xType;
    FTM_CHAR_PTR		pString;
    FTM_JSON_VALUE_PTR  pValue;
}   FTM_JSON_PAIR, * FTM_JSON_PAIR_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_INT				nMaxCount;
    FTM_INT				nCount;
    FTM_JSON_PAIR_PTR   pPairs[];
}   FTM_JSON_OBJECT, _PTR_ FTM_JSON_OBJECT_PTR;

typedef struct  
{
    FTM_JSON_TYPE       xType;
    FTM_INT				nMaxCount;
    FTM_INT				nCount;
    FTM_JSON_VALUE_PTR  pElements[];
}   FTM_JSON_ARRAY, _PTR_ FTM_JSON_ARRAY_PTR;

FTM_RET  FTM_JSON_createString
(
	FTM_CHAR_PTR	pString, 
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createNumber
(
	FTM_INT		nValue,
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createHex
(
	FTM_INT		nValue,
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createFloat
(
	FTM_FLOAT	fValue,
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createPair
(
	FTM_CHAR_PTR	pString,
	FTM_JSON_VALUE_PTR pValue,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
);

FTM_RET  FTM_JSON_createObject
(
	FTM_ULONG	ulMaxCount,
	FTM_JSON_OBJECT_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createArray
(	
	FTM_ULONG	ulMaxCount,
	FTM_JSON_ARRAY_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createTrue
(
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createFalse
(
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET  FTM_JSON_createNull
(
	FTM_JSON_VALUE_PTR _PTR_ ppItem
);

FTM_RET	FTM_JSON_destroy
(
	FTM_JSON_VALUE_PTR _PTR_ pValue
);

FTM_RET	FTM_JSON_OBJECT_setPair
(
	FTM_JSON_OBJECT_PTR pObject, 
	FTM_CHAR_PTR		pString, 
	FTM_JSON_VALUE_PTR 	pValue
);

FTM_RET	FTM_JSON_ARRAY_setElement
(
	FTM_JSON_ARRAY_PTR 	pArray, 
	FTM_JSON_VALUE_PTR pElement
);

FTM_RET	FTM_JSON_snprint
(
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulBuffLen, 
	FTM_JSON_VALUE_PTR 	pValue
);

FTM_RET	FTM_JSON_buffSize
(
	FTM_JSON_VALUE_PTR 	pValue,
	FTM_ULONG_PTR		pulLen
);
#endif
