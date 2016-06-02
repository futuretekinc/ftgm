#include <stdio.h>
#include <curl/curl.h>
#include "ftom.h"
#include "ftom_tp_client.h"

static 
size_t FTOM_TP_CLIENT_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
);

static
FTM_RET	FTOM_TP_CLIENT_setURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static FTM_BOOL	bGlobalInit = FTM_FALSE;

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (!bGlobalInit)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		bGlobalInit = FTM_TRUE;	
	}

	memset(pClient, 0, sizeof(FTOM_TP_CLIENT));

  	pClient->pCURL = curl_easy_init();
  	if(pClient->pCURL == NULL) 
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pClient->pBase, "https://api.thingplus.net/v1");
	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "username:00405cabcdef");
	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "apikey:tlLZy-8UeYAzNMubWvQWS19RUV4=");

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPHEADER, pClient->pHTTPHeader);
 	curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEFUNCTION, FTOM_TP_CLIENT_CB_response);
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEDATA, (FTM_VOID_PTR)pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->pCURL != NULL)
	{
    	curl_easy_cleanup(pClient->pCURL);
		pClient->pCURL = NULL;
	}

	if (pClient->pHTTPHeader != NULL)
	{
		curl_slist_free_all(pClient->pHTTPHeader);
		pClient->pHTTPHeader = NULL;		
	}

	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
		pClient->pResp = NULL;
	}
	if (bGlobalInit)
	{
  		curl_global_cleanup();

		bGlobalInit = FTM_FALSE;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_getGatewayInfo
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	CURLcode res;

	FTOM_TP_CLIENT_setURL(pClient, "/gateways/00405cabcdef?fields=model&fiedlds=autoCreateDiscoverable");

    res = curl_easy_perform(pClient->pCURL);
    if(res != CURLE_OK)
	{
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_getGatewayModel
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTM_ULONG			ulModel
)
{
	CURLcode res;

	FTOM_TP_CLIENT_setURL(pClient, "/gatewayModels/%lu", ulModel);

    res = curl_easy_perform(pClient->pCURL);
    if(res != CURLE_OK)
	{
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}

	printf("%s", pClient->pResp);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_setURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->pBase);

	va_start(pArgs, pFormat);
	vsprintf(&pClient->pURL[strlen(pClient->pURL)], pFormat, pArgs);
	va_end(pArgs);

	curl_easy_setopt(pClient->pCURL, CURLOPT_URL, pClient->pURL);
	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
	}

	pClient->pResp = FTM_MEM_malloc(1);
	pClient->ulRespLen = 0;
	

	return	FTM_RET_OK;
}

size_t FTOM_TP_CLIENT_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
)
{
	ASSERT(pContents != NULL);
	ASSERT(pUser != NULL);

	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pUser;
	FTM_INT				nRealSize = nSize * nMemB;
	FTM_CHAR_PTR		pMem = NULL;

	if (nRealSize == 0)
	{
		return	0;	
	}

	pMem = (FTM_VOID_PTR)FTM_MEM_malloc(pClient->ulRespLen + nRealSize + 1);
	if (pMem == NULL)
	{
		ERROR("Not enough memory!\n");
		return	0;	
	}

	strcpy(pMem, pClient->pResp);
	memcpy(&pMem[pClient->ulRespLen], pContents, nRealSize);
	FTM_MEM_free(pClient->pResp);
	pClient->pResp = pMem;

	pClient->ulRespLen += nRealSize;
	pClient->pResp[pClient->ulRespLen] = 0;
	return nRealSize;
}

int main(void)
{
		FTM_RET			xRet;
		FTOM_TP_CLIENT	xClient;

	xRet = FTOM_TP_CLIENT_init(&xClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Thingplus client initialization failed.\n");
		return	0;	
	}

	FTOM_TP_CLIENT_getGatewayInfo(&xClient);
	FTOM_TP_CLIENT_getGatewayModel(&xClient, 4);


	FTOM_TP_CLIENT_final(&xClient);

  	return 0;
}
