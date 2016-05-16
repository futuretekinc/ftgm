#ifndef	_FTOM_CGI_CMD_DISCOVERY_H_
#define	_FTOM_CGI_CMD_DISCOVERY_H_

FTM_RET	FTOM_CGI_startDiscovery
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getDiscoveryInfo
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getDiscoveryNodeList
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTOM_CGI_getDiscoveryEPList
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

#endif
