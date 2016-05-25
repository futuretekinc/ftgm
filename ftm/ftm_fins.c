#include <string.h>
#include "ftm_trace.h"
#include "ftm_fins.h"

static 
FTM_FINS_REGISTER_MAP	pHHTWCompMaps[] =
{
	{
		.ulID		=	0x0C000001,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0C000002,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0C000003,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0C000004,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0C000005,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0A000001,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	},
	{
		.ulID		=	0x0A000002,
		.ulOffset	=	0,
		.xValueType	=	FTM_VALUE_TYPE_FLOAT,
		.ulDevider	=	100
	}
};

static
FTM_FINS_DESCRIPT	pFINSDescriptList[] =
{
	{
		.pModel			= "hhtw comp",
		.ulBaseAddress	=  9300,
		.ulValidRange	=  14,
		.ulRegisterCount=  7,
		.pRegisterMaps	= pHHTWCompMaps
	}
};

FTM_RET	FTM_FINS_get
(
	FTM_CHAR_PTR	pModel,
	FTM_FINS_DESCRIPT_PTR	_PTR_ pFINS
)
{
	ASSERT(pModel != NULL);
	ASSERT(pFINS != NULL);
	FTM_INT	i;

	for(i = 0 ; i < sizeof(pFINSDescriptList) / sizeof(FTM_FINS_DESCRIPT) ; i++)
	{
		if (strcasecmp(pModel, pFINSDescriptList[i].pModel) == 0)
		{
			*pFINS = &pFINSDescriptList[i];	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}
