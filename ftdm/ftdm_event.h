#ifndef	_FTDM_TRIGGER_H_
#define	_FTDM_TRIGGER_H_

#include "ftm.h"

FTM_RET	FTDM_TRIGGER_init(FTM_VOID);
FTM_RET FTDM_TRIGGER_final(FTM_VOID);
FTM_RET	FTDM_TRIGGER_showList(FTM_VOID);
FTM_RET	FTDM_TRIGGER_loadFromFile(FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_TRIGGER_loadFromDB(FTM_VOID);
FTM_RET	FTDM_TRIGGER_add(FTM_TRIGGER_PTR pInfo);
FTM_RET	FTDM_TRIGGER_del(FTM_EP_ID	xEPID);
FTM_RET	FTDM_TRIGGER_count(FTM_ULONG_PTR	pnCount);
FTM_RET	FTDM_TRIGGER_get(FTM_EP_ID	xEPID, FTM_TRIGGER_PTR _PTR_ 	ppEPInfo);
FTM_RET	FTDM_TRIGGER_getAt(FTM_ULONG	nIndex, FTM_TRIGGER_PTR _PTR_	ppEPInfo);

#endif
