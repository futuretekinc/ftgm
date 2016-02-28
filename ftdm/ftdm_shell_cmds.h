#ifndef	_FTDM_SHELL_CMDS_H_
#define	_FTDM_SHELL_CMDS_H_

#include "ftm_types.h"
#include "ftm_node.h"
#include "ftm_ep.h"

FTM_RET	FTDM_SHELL_showNodeList(void);
FTM_RET	FTDM_SHELL_showNodeInfo(FTM_CHAR_PTR pDID);
FTM_RET	FTDM_SHELL_showEPList(void);
FTM_RET	FTDM_SHELL_showEPInfo(FTM_EP_ID xEPID);
FTM_RET	FTDM_SHELL_showEPData(FTM_EP_ID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount);

#endif

