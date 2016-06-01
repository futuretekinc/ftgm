#include "ftom_node_class.h"
#include "ftom_node_snmp_client.h"
#include "ftom_node_fins_client.h"

FTM_RET	FTOM_NODE_CLASS_get
(
	FTM_CHAR_PTR			pModel,
	FTM_NODE_TYPE			xType,
	FTOM_NODE_CLASS_PTR _PTR_	ppClass
)
{
	ASSERT(pModel != NULL);
	ASSERT(ppClass != NULL);

	FTM_RET	xRet;

	TRACE("Node class type : %d\n", xType);
	switch(xType)
	{
	case	FTM_NODE_TYPE_SNMP: xRet = FTOM_NODE_SNMPC_getClass(pModel, ppClass); break;
	case	FTM_NODE_TYPE_FINS: xRet = FTOM_NODE_FINSC_getClass(pModel, ppClass); break;
	default:	xRet = FTM_RET_OBJECT_NOT_FOUND;
	}

	return	xRet;
}
