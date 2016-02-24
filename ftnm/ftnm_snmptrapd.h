#ifndef	_FTNM_SNMPTRAPD_H_
#define	_FTNM_SNMPTRAPD_H_

#include "ftm_types.h"

typedef struct
{
	FTM_CHAR	pName[128];
	FTM_USHORT	usPort;
}	FTNM_SNMPTRAPD_CONFIG, _PTR_ FTNM_SNMPTRAPD_CONFIG_PTR;
	
typedef	struct
{
	FTNM_SNMPTRAPD_CONFIG	xConfig;
	FTM_BOOL				bRunning;
}	FTNM_SNMPTRAPD, _PTR_ FTNM_SNMPTRAPD_PTR;

#endif
