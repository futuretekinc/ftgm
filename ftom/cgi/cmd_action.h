#ifndef	_FTOM_CGI_CMD_ACTION_H_
#define	_FTOM_CGI_CMD_ACTION_H_

FTM_RET	FTOM_CGI_addAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getActionList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
