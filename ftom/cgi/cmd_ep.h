#ifndef	_FTOM_CGI_CMD_EP_H_
#define	_FTOM_CGI_CMD_EP_H_

FTM_RET	FTOM_CGI_addEP
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setEP
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getEPList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setEPList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getEPDataInfo
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getEPData
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_setEPData
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getEPDataLast
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_delEPData
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
