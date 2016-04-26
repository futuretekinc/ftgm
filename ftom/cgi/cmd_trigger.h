#ifndef	_FTOM_CGI_CMD_TRIGGER_H_
#define	_FTOM_CGI_CMD_TRIGGER_H_

FTM_RET	FTOM_CGI_getTrigger
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_addTrigger
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delTrigger
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getTriggerList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
