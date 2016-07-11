#include <stdio.h>
#include "ftm_types.h"
#include "ftm_node.h"

FTM_RET	FTM_isValidDID(FTM_CHAR_PTR pDID)
{
	FTM_INT	i;

	if ((pDID == '\0') || (pDID[0] == '\0'))
	{
		return	FTM_RET_INVALID_DID;	
	}

	for(i = 1 ; i < FTM_DID_LEN + 1 ; i++)
	{
		if (pDID[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_DID;	
}

FTM_RET	FTM_isValidName(FTM_CHAR_PTR pName)
{
	FTM_INT	i;

	if (pName == '\0')
	{
		return	FTM_RET_INVALID_NAME;	
	}

	for(i = 0 ; i < FTM_NAME_LEN ; i++)
	{
		if (pName[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_NAME;	
}

FTM_RET	FTM_isValidUnit(FTM_CHAR_PTR pUnit)
{
	FTM_INT	i;

	if (pUnit == '\0')
	{
		return	FTM_RET_INVALID_UNIT;	
	}

	for(i = 0 ; i < FTM_UNIT_LEN ; i++)
	{
		if (pUnit[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_UNIT;	
}

FTM_RET	FTM_isValidLocation(FTM_CHAR_PTR pLocation)
{
	FTM_INT	i;

	if (pLocation == '\0')
	{
		return	FTM_RET_INVALID_LOCATION;	
	}

	for(i = 0 ; i < FTM_LOCATION_LEN ; i++)
	{
		if (pLocation[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_LOCATION;	
}

FTM_RET	FTM_isValidURL(FTM_CHAR_PTR pURL)
{
	FTM_INT	i;

	if (pURL == '\0')
	{
		return	FTM_RET_INVALID_URL;	
	}

	for(i = 0 ; i < FTM_URL_LEN ; i++)
	{
		if (pURL[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_URL;	
}

FTM_RET	FTM_isValidCommunity(FTM_CHAR_PTR pCommunity)
{
	FTM_INT	i;

	if (pCommunity == '\0')
	{
		return	FTM_RET_INVALID_COMMUNITY;	
	}

	for(i = 0 ; i < FTM_SNMP_COMMUNITY_LEN ; i++)
	{
		if (pCommunity[i] == '\0')
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_COMMUNITY;	
}

FTM_RET	FTM_isValidInterval(FTM_ULONG ulInterval)
{
	if ((ulInterval < FTM_INTERVAL_MIN) || (FTM_INTERVAL_MAX < ulInterval))
	{
		return	FTM_RET_INVALID_INTERVAL;	
	}

	return	FTM_RET_OK;
}
