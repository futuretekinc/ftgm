#include "ftdm.h"

FTM_RET	FTDM_LOG_create
(
	FTM_LOG_PTR	pLog
)
{
	return	FTM_LOG_add(pLog);
}
