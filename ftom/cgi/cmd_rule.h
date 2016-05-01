#ifndef	_FTOM_CGI_CMD_RULE_H_
#define	_FTOM_CGI_CMD_RULE_H_

FTM_RET	FTOM_CGI_getRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_addRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getRuleList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
