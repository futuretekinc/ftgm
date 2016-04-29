#ifndef	_FTOM_CGI_CMD_NODE_H_
#define	_FTOM_CGI_CMD_NODE_H_

FTM_RET	FTOM_CGI_addNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getNodeList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);
#endif
