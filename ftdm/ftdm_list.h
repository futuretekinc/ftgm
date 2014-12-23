#ifndef	__FTDM_LIST_H__
#define	__FTDM_LIST_H__

typedef	struct _FTDM_LIST_NODE
{
	struct _FTDM_LIST_NODE *pPrev;	
	struct _FTDM_LIST_NODE *pNext;	
}	FTDM_LIST_NODE, _PTR_ FTDM_LIST_NODE_PTR;

typedef	struct
{
	FTDM_LIST_NODE	xHead;
	FTDM_ULONG		nCount;
}	FTDM_LIST, _PTR_ FTDM_LIST_PTR;

FTDM_RET	FTDM_LIST_init(FTDM_LIST_PTR pList);
FTDM_RET	FTDM_LIST_pushback(FTDM_LIST_PTR pList, FTDM_LIST_NODE_PTR pNode);
FTDM_RET	FTDM_LIST_remove(FTDM_LIST_PTR pList, FTDM_LIST_NODE_PTR pNode);

#endif
