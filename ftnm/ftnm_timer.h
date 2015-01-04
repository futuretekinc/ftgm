#ifndef	__FTNM_TIMER_H__
#define	__FTNM_TIMER_H__

#include "ftm_types.h"

typedef	enum
{
	FTNM_TIMER_TYPE_RELATIVE=0,
	FTNM_TIMER_TYPE_ABSOLUTE=1
}	FTNM_TIMER_TYPE, _PTR_ FTNM_TIMER_TYPE_PTR;

typedef	struct
{
	FTNM_TIMER_TYPE		xType;

}	FTNM_TIMER, _PTR_ FTNM_TIMER_PTR;

#endif

