#ifndef	_FTM_OM_PLUGIN_SERVER_H_
#define	_FTM_OM_PLUGIN_SERVER_H_

#include "ftm.h"

FTM_RET	(*FTM_OM_PLUGIN_INIT)(FTM_VOID_PTR pData);
FTM_RET	(*FTM_OM_PLUGIN_FINAL)(FTM_VOID_PTR pData);
FTM_RET (*FTM_OM_PLUGIN_CREATE)(FTM_VOID_PTR _PTR_ pInstance);
FTM_RET (*FTM_OM_PLUGIN_START)(FTM_VOID_PTR pInstance);
FTM_RET (*FTM_OM_PLUGIN_STOP)(FTM_VOID_PTR pInstance);

typedef	struct
{
	FTM_OM_PLUGIN_INIT	fInit;
	FTM_OM_PLUGIN_FINAL	fFinal;
	FTM_OM_PLUGIN_CREATE	fCreate;
	FTM_OM_PLUGIN_START	fStart;
	FTM_OM_PLUGIN_STOP	fStop;
}	FTM_OM_PLUGIN_SERVER, _PTR_ FTM_OM_PLUGIN_SERVER_PTR;

#endif