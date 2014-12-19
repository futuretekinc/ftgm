#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "debug.h"


void *FTDM_serviceHandler(void *pData);

FTDM_RET FTDM_shellCmdVersion(FTDM_CHAR_PTR pProgramName)
{
	printf("%s v1.0.0.0\n", pProgramName);

	return	FTDM_RET_OK;
}

FTDM_RET FTDM_shellCmdUsage(FTDM_CHAR_PTR pProgramName)
{
	printf("Usage : %s [OPTIONS]\n", pProgramName);
	printf("OPTIONS include:\n");
	printf("    -v           Show version\n");

	return	FTDM_RET_OK;
}


