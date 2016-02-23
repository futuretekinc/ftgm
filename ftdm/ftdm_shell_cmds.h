#ifndef	_FTDM_SHELL_CMDS_H_
#define	_FTDM_SHELL_CMDS_H_

#include "ftm_types.h"
#include "ftm_node.h"
#include "ftm_endpoint.h"

FTM_RET	FTDM_SHELL_showNodeList(void);
FTM_RET	FTDM_SHELL_showNodeInfo(FTM_CHAR_PTR pDID);
FTM_RET	FTDM_SHELL_showEPList(void);
FTM_RET	FTDM_SHELL_showEPInfo(FTM_EPID xEPID);
FTM_RET	FTDM_SHELL_showEPData(FTM_EPID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount);

#endif

