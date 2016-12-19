#ifndef	__FTM_FINS_H__
#define	__FTM_FINS_H__

#include "ftm_types.h"
#include "ftm_value.h"

typedef	struct
{
	FTM_ULONG		ulID;
	FTM_ULONG		ulOffset;
	FTM_VALUE_TYPE	xValueType;
	FTM_ULONG		ulDevider;
}	FTM_FINS_REGISTER_MAP, _PTR_ FTM_FINS_REGISTER_MAP_PTR;


typedef	struct
{
	FTM_CHAR	pModel[FTM_NAME_LEN+1];
	FTM_ULONG	ulBaseAddress;
	FTM_ULONG	ulValidRange;
	FTM_ULONG	ulRegisterCount;
	FTM_FINS_REGISTER_MAP_PTR 	pRegisterMaps;
}	FTM_FINS_DESCRIPT, _PTR_ FTM_FINS_DESCRIPT_PTR;

FTM_RET	FTM_FINS_get
(
	FTM_CHAR_PTR	pModel,
	FTM_FINS_DESCRIPT_PTR	_PTR_ pFINS
);

#endif
