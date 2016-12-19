#ifndef	_FTDM_SHELL_CMDS_H_
#define	_FTDM_SHELL_CMDS_H_

#include "ftm_types.h"
#include "ftm_node.h"
#include "ftdm_node_management.h"
#include "ftm_ep.h"
#include "ftdm_ep_management.h"

FTM_RET	FTDM_SHELL_showNodeList
(
	FTDM_NODEM_PTR pNodeM
);

FTM_RET	FTDM_SHELL_showNodeInfo
(
	FTDM_NODEM_PTR 	pNodeM, 
	FTM_CHAR_PTR 	pDID
);

FTM_RET	FTDM_SHELL_showEPList
(
	FTDM_EPM_PTR 	pEPM
);

FTM_RET	FTDM_SHELL_showEPInfo
(
	FTDM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTDM_SHELL_showEPData
(
	FTDM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulBegin, 
	FTM_ULONG 		ulCount
);

FTM_RET	FTDM_SHELL_delEPData
(
	FTDM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTM_INT 		nIndex, 
	FTM_ULONG 		ulCount
);

#endif

