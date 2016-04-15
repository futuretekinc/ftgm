#include <pthread.h>
#include "ftm.h"
#include "ftm_shared_memory.h"

typedef	struct
{
	pthread_t	xThread;
	FTM_INT		nKey;
	FTM_ULONG	ulSize;
	FTM_INT		nLoop;
	FTM_INT		nInterval;
}	FTM_SM_TEST_CHILD, _PTR_ FTM_SM_TEST_CHILD_PTR;

static 
FTM_VOID_PTR FTM_SM_TEST_child
(
	FTM_VOID_PTR pData
);

static
FTM_RET	FTM_SM_TEST_showSM
(
	FTM_INT		nChildID,
	FTM_SM_PTR	pSM
);


FTM_SM_TEST_CHILD	xChilds[] =
{
	{
		.nKey 		= 1234,
		.ulSize 	= 1024,
		.nLoop 		= 20,
		.nInterval	= 1
	},
	{
		.nKey 		= 1234,
		.ulSize 	= 1024,
		.nLoop 		= 10,
		.nInterval	= 2
	},
	{
		.nKey 		= 1234,
		.ulSize 	= 1024,
		.nLoop 		= 7,
		.nInterval	= 3
	},
	{
		.nKey 		= 1234,
		.ulSize 	= 1024,
		.nLoop 		= 5,
		.nInterval	= 4
	},
};

int main(int argc, char *argv[])
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_SM_PTR	pSM;
	FTM_SM_PTR	pSMClone;

	xRet = FTM_SM_create(1234, 1024, &pSM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Shared memory creation failed[%08x].\n", xRet);	
		return	0;
	}

	
	for(i = 0 ; i < 4 ; i++)
	{
		pthread_create(&xChilds[i].xThread, NULL, FTM_SM_TEST_child, &xChilds[i]);
	}
	
	
	for(i = 0 ; i < 4 ; i++)
	{
		pthread_join(xChilds[i].xThread, NULL);
	}

	pSMClone = pSM;
	while(pSM->ulReference > 1)
	{
		xRet = FTM_SM_destroy(&pSM);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Shared memory destroy failed[%08x].\n", xRet);	
		}
		pSM = pSMClone;
	}

	xRet = FTM_SM_destroy(&pSM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Shared memory destroy failed[%08x].\n", xRet);	
	}

	return	0;
}


FTM_VOID_PTR FTM_SM_TEST_child(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_SM_TEST_CHILD_PTR	pChild = (FTM_SM_TEST_CHILD_PTR)pData;
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_SM_PTR	pSM;	

	MESSAGE("child[%d] start\n", pChild->xThread);
	for(i = 0 ; i < pChild->nLoop ; i++)
	{
		xRet = FTM_SM_create(pChild->nKey, pChild->ulSize, &pSM);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Shared memory creation failed[%08x].\n", xRet);	
		}
		else
		{
			MESSAGE("Shared memory creation success.\n");	
		}

		FTM_SM_TEST_showSM(pChild->xThread,pSM);
		sleep(pChild->nInterval);			

		xRet = FTM_SM_destroy(&pSM);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Shared memory destroy failed[%08x].\n", xRet);	
		}
		else
		{
			MESSAGE("Shared memory destroy success.\n");	
		}
		sleep(pChild->nInterval);			
	}
	MESSAGE("child[%d] stop\n", pChild->xThread);

	return	0;
}

FTM_RET	FTM_SM_TEST_showSM
(
	FTM_INT		nID,
	FTM_SM_PTR	pSM
)
{
	MESSAGE("Shared Memory Information\n");
	MESSAGE("%16s : %d\n", 	"Child",	nID);
	MESSAGE("%16s : %d\n", 	"Key", 		pSM->xKey);
	MESSAGE("%16s : %d\n", 	"Size", 	pSM->ulSize);
	MESSAGE("%16s : %08x\n","Address", 	pSM->pMem);
	MESSAGE("%16s : %d\n",	"Reference",pSM->ulReference);

	return	FTM_RET_OK;
}
