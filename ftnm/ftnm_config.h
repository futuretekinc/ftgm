#ifndef	__FTNM_CONFIG_H__
#define	__FTNM_CONFIG_H__

#include "ftm_types.h"
#include "simclist.h"

typedef	struct
{
	FTM_CHAR_PTR	pType;
	FTM_CHAR_PTR	pOIDPrefix;

}	FTNM_EP_INFO, _PTR_ FTNM_EP_INFO_PTR;

typedef	struct
{
	FTM_CHAR_PTR	pName;
	list_t			xEPList;
}	FTNM_CONFIG, _PTR_ FTNM_CONFIG_PTR;

FTM_RET FTNM_configInit(void);
FTM_RET	FTNM_configFinal(void);
FTM_RET FTNM_configLoad(FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_getAppName(FTM_CHAR_PTR pBuff, FTM_INT nLen);
FTM_RET	FTNM_getEPInfoCount(void);
FTM_RET	FTNM_getEPInfo(FTM_INT nIndex, FTNM_EP_INFO_PTR pInfo);
#endif
