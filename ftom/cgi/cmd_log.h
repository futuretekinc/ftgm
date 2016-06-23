#ifndef	_FTOM_CGI_CMD_LOG_H_
#define	_FTOM_CGI_CMD_LOG_H_

FTM_RET	FTOM_CGI_getLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_addLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getLogList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
