#ifndef	_FTOM_CGI_H_
#define	_FTOM_CGI_H_

#include <syslog.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_client.h"
#include "qdecoder.h"
#include "mxml.h"

typedef	struct FTOM_CGI_COMMAND_STRUCT
{
	FTM_CHAR_PTR	pName;
	FTM_RET			(*fService)(FTOM_CLIENT_PTR pClient, qentry_t *req);
} FTOM_CGI_COMMAND, _PTR_ FTOM_CGI_COMMAND_PTR;

FTM_RET	FTOM_CGI_main(FTOM_CLIENT_PTR pClient, qentry_t *pReq);

const 
char *FTOM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
);

#undef	TRACE
#define	TRACE(format, ...) 	syslog(LOG_INFO, format, ## __VA_ARGS__)

#endif
