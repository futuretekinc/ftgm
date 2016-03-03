#ifndef	_FTDM_ACT_H_
#define	_FTDM_ACT_H_

#include "ftm_types.h"
#include "ftm_list.h"
#include "ftm_act.h"

FTM_RET	FTDM_ACT_init(FTM_VOID);
FTM_RET FTDM_ACT_final(FTM_VOID);
FTM_RET	FTDM_ACT_showList(FTM_VOID);
FTM_RET	FTDM_ACT_loadFromFile(FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_ACT_loadFromDB(FTM_VOID);
FTM_RET	FTDM_ACT_add(FTM_ACT_PTR pInfo);
FTM_RET	FTDM_ACT_del(FTM_EPID	xEPID);
FTM_RET	FTDM_ACT_count(FTM_ULONG_PTR	pnCount);
FTM_RET	FTDM_ACT_get(FTM_EPID	xEPID, FTM_ACT_PTR _PTR_ 	ppEPInfo);
FTM_RET	FTDM_ACT_getAt(FTM_ULONG	nIndex, FTM_ACT_PTR _PTR_	ppEPInfo);

#endif
