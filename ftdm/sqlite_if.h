#ifndef	__SQLITE_IF_H__
#define	__SQLITE_IF_H__

FTM_RET	FTDM_DBIF_init
(
	void
);

FTM_RET	FTDM_DBIF_final
(
	void
);

FTM_RET	FTDM_DBIF_initNodeInfoTable
(
	void
);

FTM_RET	FTDM_DBIF_getNodeCount
(
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_getNodeList
(
	FTM_NODE_INFO_PTR	pInfos,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_insertNodeInfo
(
 	FTM_NODE_INFO_PTR	pInfo
);

FTM_RET	FTDM_DBIF_removeNodeInfo
(
	FTM_CHAR_PTR		pDID
);

FTM_RET	FTDM_DBIF_getNodeInfo
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_INFO_PTR	pInfo
);

FTM_RET	FTDM_DBIF_setNodeURL
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pURL
);

FTM_RET	FTDM_DBIF_getNodeURL
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				nBuffLen
);

FTM_RET	FTDM_DBIF_setNodeLocation
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pLocation
);

FTM_RET	FTDM_DBIF_getNodeLocation
(
	FTM_CHAR_PTR			pDID,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				nBuffLen
);

FTM_RET	FTDM_DBIF_initEPInfoTable
(
	void
);

FTM_RET	FTDM_DBIF_insertEPInfo
(
 	FTM_EP_INFO_PTR		pInfo
);

FTM_RET	FTDM_DBIF_removeEPInfo
(
	FTM_EPID			xEPID
);

FTM_RET	FTDM_DBIF_getEPCount
(
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDM_DBIF_getEPList
(
	FTM_EP_INFO_PTR		pInfos,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDM_DBIF_getEPInfo
(
	FTM_EPID 				xEPID, 
	FTM_EP_INFO_PTR		pInfo
);

FTM_RET	FTDM_DBIF_setEPName
(
	FTM_EPID				xEPID,
	FTM_CHAR_PTR			pName,
	FTM_INT				nNameLen
);

FTM_RET	FTDM_DBIF_getEPName
(
	FTM_EPID			xEPID,
	FTM_CHAR_PTR		pName,
	FTM_INT_PTR			pNameLen
);

FTM_RET	FTDM_DBIF_setEPInterval
(
	FTM_EPID			xEPID,
	FTM_ULONG			nInterval
);

FTM_RET	FTDM_DBIF_getEPInterval
(
	FTM_EPID			xEPID,
	FTM_ULONG_PTR		pInterval
);

FTM_RET	FTDM_DBIF_setEPUnit
(
	FTM_EPID			xEPID,
	FTM_CHAR_PTR		pUnit,
	FTM_INT				nUnitLen
);

FTM_RET	FTDM_DBIF_getEPUnit
(
	FTM_EPID			xEPID,
	FTM_CHAR_PTR		pUnit,
	FTM_INT_PTR			pUnitLen
);


FTM_RET	FTDM_DBIF_initEPDataTable
(
	FTM_EPID			xEPID
);

FTM_RET	FTDM_DBIF_appendEPData
(
	FTM_EPID			xEPID,
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTDM_DBIF_EPDataCount
(
	FTM_EPID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_getEPData
(
	FTM_EPID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_getEPDataWithTime
(
	FTM_EPID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_DBIF_removeEPData
(
	FTM_EPID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
);

FTM_RET	FTDM_DBIF_removeEPDataWithTime
(
	FTM_EPID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime
);

#endif
