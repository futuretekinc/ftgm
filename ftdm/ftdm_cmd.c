#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "debug.h"


void *FTDM_serviceHandler(void *pData);

FTM_RET FTDM_shellCmdVersion(FTM_CHAR_PTR pProgramName)
{
	printf("%s v1.0.0.0\n", pProgramName);

	return	FTM_RET_OK;
}

FTM_RET FTDM_shellCmdUsage(FTM_CHAR_PTR pProgramName)
{
	printf("Usage : %s [OPTIONS]\n", pProgramName);
	printf("OPTIONS include:\n");
	printf("    -v           Show version\n");

	return	FTM_RET_OK;
}


