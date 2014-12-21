#ifndef	__FTDM_EP_H__
#define	__FTDM_EP_H__

typedef	struct _FTDM_EP
{
	struct _FTDM_EP *pPrev;
	struct _FTDM_EP *pNext;

	FTDM_EP_INFO	xInfo;
}	FTDM_EP, _PTR_ FTDM_EP_PTR;

#endif
