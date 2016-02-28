#ifndef	_FTNM_MSG_H_
#define	_FTNM_MSG_H_

#include "ftm_ep.h"
#define	FTNM_MSG_STRING_LENGTH	1024
typedef	enum
{
	FTNM_MSG_TYPE_SNMPTRAP = 0,
	FTNM_MSG_TYPE_EP_CHANGED

}	FTNM_MSG_TYPE, _PTR_ FTNM_MSG_TYPE_PTR;

typedef	struct
{
	FTM_CHAR	pString[FTNM_MSG_STRING_LENGTH+1];	
}	FTNM_MSG_SNMPTRAP_PARAMS, _PTR_ FTNM_MSG_SNMPTRAP_PARAMS_PTR;

typedef struct
{
	FTM_EP_ID	xEPID;
	FTM_EP_DATA	xData;
}	FTNM_MSG_EP_CHANGED_PARAMS, _PTR_ FTNM_MSG_EP_CHANGED_PARAMS_PTR;

typedef	struct
{
	FTNM_MSG_TYPE	xType;
	union
	{
		FTNM_MSG_SNMPTRAP_PARAMS	xSNMPTrap;
		FTNM_MSG_EP_CHANGED_PARAMS	xEPChanged;
	}	xParams;
} FTNM_MSG, _PTR_ FTNM_MSG_PTR;

FTM_RET FTNM_MSG_init(FTM_VOID);
FTM_RET FTNM_MSG_final(FTM_VOID);

FTM_RET	FTNM_MSG_pop(FTNM_MSG_PTR _PTR_ ppMsg);
FTM_RET	FTNM_MSG_sendSNMPTrap(FTM_CHAR_PTR pTrapMsg);
FTM_RET FTNM_MSG_sendEPChanged(FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);

#endif
