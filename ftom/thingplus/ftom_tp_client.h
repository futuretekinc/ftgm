#ifndef	__FTOM_TP_CLIENT_H__
#define	__FTOM_TP_CLIENT_H__

#include "ftom.h"
#include <curl/curl.h>

typedef	struct
{
	CURL 				_PTR_ pCURL;
	struct curl_slist	_PTR_ pHTTPHeader;

	FTM_CHAR			pBase[1024];
	FTM_CHAR			pURL[1024];
	FTM_CHAR_PTR		pResp;
	FTM_ULONG			ulRespLen;
}	FTOM_TP_CLIENT, _PTR_ FTOM_TP_CLIENT_PTR;

#endif
