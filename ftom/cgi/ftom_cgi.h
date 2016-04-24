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

FTM_RET	FTOM_CGI_node
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_ep
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_data
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

const 
char *FTOM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
);

FTM_RET	FTOM_CGI_getEPID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pEPID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getEPType
(
	qentry_t *pReq, 
	FTM_EP_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getEPFlags
(
	qentry_t *pReq, 
	FTM_EP_FLAG_PTR	pFlags,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getName
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getUnit
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pUnit,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pEnable	,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getTimeout
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pTimeout,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getDID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pDID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getLimit
(
	qentry_t *pReq,
	FTM_EP_LIMIT_PTR pLimit,
	FTM_BOOL	bAllowEmpty
);

#undef	TRACE
#define	TRACE(format, ...) 	syslog(LOG_INFO, format, ## __VA_ARGS__)

#endif
