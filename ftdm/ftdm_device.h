#ifndef	__FTDM_DEVICE_H__
#define	__FTDM_DEVICE_H__

#include "ftdm_type.h"

#define	FTDM_DT_ROOT	0x00000001

typedef	struct _FTDM_DEVICE
{
	struct _FTDM_DEVICE	* pPrev;
	struct _FTDM_DEVICE	* pNext;
	FTDM_DEVICE_INFO	xInfo;
}	FTDM_DEVICE, _PTR_ FTDM_DEVICE_PTR;

FTDM_RET	FTDM_initDevice
(
 	FTDM_VOID
);

FTDM_RET	FTDM_finalDevice
(
 	FTDM_VOID
);

FTDM_RET	FTDM_insertDevice
(
 	FTDM_DEVICE_PTR pDevice
);

FTDM_RET	FTDM_removeDevice
(
 	FTDM_DEVICE_PTR pDevice
);

#endif

