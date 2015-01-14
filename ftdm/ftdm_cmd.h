#ifndef	__FTDM_CMD_H__
#define	__FTDM_CMD_H__

typedef	unsigned long	FTDM_CMD, _PTR_ FTDM_CMD_PTR;

#define	FTDM_CMD_UNKNOWN						0x00000000

#define	FTDM_CMD_ADD_NODE_INFO					0x01000001
#define	FTDM_CMD_DEL_NODE_INFO					0x01000002
#define	FTDM_CMD_GET_NODE_INFO_COUNT			0x01000003
#define	FTDM_CMD_GET_NODE_INFO					0x01000004
#define	FTDM_CMD_GET_NODE_INFO_BY_INDEX			0x01000005
#define	FTDM_CMD_GET_NODE_TYPE					0x01000006
#define	FTDM_CMD_GET_NODE_URL					0x01000007
#define	FTDM_CMD_SET_NODE_URL					0x01000008
#define	FTDM_CMD_GET_NODE_LOCATION				0x01000009
#define	FTDM_CMD_SET_NODE_LOCATION				0x0100000A

#define	FTDM_CMD_ADD_EP_INFO					0x02000001
#define	FTDM_CMD_DEL_EP_INFO					0x02000002
#define	FTDM_CMD_GET_EP_INFO_COUNT				0x02000003
#define	FTDM_CMD_GET_EP_INFO					0x02000004
#define	FTDM_CMD_GET_EP_INFO_BY_INDEX			0x02000005

#define	FTDM_CMD_ADD_EP_CLASS_INFO				0x03000001
#define	FTDM_CMD_DEL_EP_CLASS_INFO				0x03000002
#define	FTDM_CMD_GET_EP_CLASS_INFO_COUNT		0x03000003
#define	FTDM_CMD_GET_EP_CLASS_INFO				0x03000004
#define	FTDM_CMD_GET_EP_CLASS_INFO_BY_INDEX		0x03000005

#define	FTDM_CMD_ADD_EP_DATA					0x04000001
#define	FTDM_CMD_DEL_EP_DATA					0x04000002
#define	FTDM_CMD_DEL_EP_DATA_WITH_TIME			0x04000003
#define	FTDM_CMD_GET_EP_DATA					0x04000004
#define	FTDM_CMD_GET_EP_DATA_WITH_TIME			0x04000005
#define	FTDM_CMD_GET_EP_DATA_COUNT				0x04000006
#define	FTDM_CMD_GET_EP_DATA_COUNT_WITH_TIME	0x04000007

#endif
