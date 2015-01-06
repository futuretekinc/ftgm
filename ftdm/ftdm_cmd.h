#ifndef	__FTDM_CMD_H__
#define	__FTDM_CMD_H__

#define	FTDM_CMD_UNKNOWN					0x00000000

#define	FTDM_CMD_CREATE_NODE				0x01000001
#define	FTDM_CMD_DESTROY_NODE				0x01000002
#define	FTDM_CMD_GET_NODE_COUNT				0x01000003
#define	FTDM_CMD_GET_NODE_INFO				0x01000004
#define	FTDM_CMD_GET_NODE_INFO_BY_INDEX		0x01000005
#define	FTDM_CMD_GET_NODE_TYPE				0x01000006
#define	FTDM_CMD_GET_NODE_URL				0x01000007
#define	FTDM_CMD_SET_NODE_URL				0x01000008
#define	FTDM_CMD_GET_NODE_LOCATION			0x01000009
#define	FTDM_CMD_SET_NODE_LOCATION			0x0100000A

#define	FTDM_CMD_CREATE_EP					0x02000001
#define	FTDM_CMD_DESTROY_EP					0x02000002
#define	FTDM_CMD_GET_EP_COUNT				0x02000003
#define	FTDM_CMD_GET_EP_INFO				0x02000004
#define	FTDM_CMD_GET_EP_INFO_BY_INDEX		0x02000005
#define	FTDM_CMD_APPEND_EP_DATA				0x02000006
#define	FTDM_CMD_GET_EP_DATA				0x02000007
#define	FTDM_CMD_GET_EP_DATA_WITH_TIME		0x02000008
#define	FTDM_CMD_REMOVE_EP_DATA				0x02000009
#define	FTDM_CMD_REMOVE_EP_DATA_WITH_TIME	0x0200000A

#endif
