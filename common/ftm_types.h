#ifndef	__FTM_TYPES_H__
#define	__FTM_TYPES_H__

#include <stdint.h>
#include <linux/types.h>

#define	_PTR_	*

#define	FTM_VOID		void
#define	FTM_VOID_PTR	void *
typedef	char			FTM_CHAR, 	_PTR_ FTM_CHAR_PTR;
typedef	char			FTM_INT8, 	_PTR_ FTM_INT8_PTR;
typedef	unsigned char	FTM_BYTE, 	_PTR_ FTM_BYTE_PTR;
typedef	unsigned char	FTM_UINT8, 	_PTR_ FTM_UINT8_PTR;
typedef	unsigned short 	FTM_USHORT, _PTR_ FTM_USHORT_PTR;
typedef	int				FTM_INT, 	_PTR_ FTM_INT_PTR;
typedef	int16_t			FTM_INT16,  _PTR_ FTM_INT16_PTR;
typedef	int32_t			FTM_INT32, 	_PTR_ FTM_INT32_PTR;
typedef	int64_t			FTM_INT64,  _PTR_ FTM_INT64_PTR;
typedef	unsigned int	FTM_UINT, 	_PTR_ FTM_UINT_PTR;
typedef	uint16_t		FTM_UINT16, _PTR_ FTM_UINT16_PTR;
typedef	uint32_t		FTM_UINT32, _PTR_ FTM_UINT32_PTR;
typedef	uint64_t		FTM_UINT64, _PTR_ FTM_UINT64_PTR;
typedef	long			FTM_LONG, 	_PTR_ FTM_LONG_PTR;
typedef	unsigned long	FTM_ULONG, 	_PTR_ FTM_ULONG_PTR;
typedef	int				FTM_BOOL, 	_PTR_ FTM_BOOL_PTR;
typedef	float			FTM_FLOAT, 	_PTR_ FTM_FLOAT_PTR;
typedef	double			FTM_DOUBLE, _PTR_ FTM_DOUBLE_PTR;

typedef	const char _PTR_ FTM_CONST_CHAR_PTR;

#define	FTM_FALSE		0
#define	FTM_TRUE		(!FTM_FALSE)

typedef	unsigned long	FTM_IP_ADDR, _PTR_ FTM_IP_ADDR_PTR;

typedef	unsigned long	FTM_CMD, _PTR_ FTM_CMD_PTR;

#define	FTM_FILE_NAME_LEN					256

typedef	unsigned long	FTM_RET, _PTR_ FTM_RET_PTR;

#define	FTM_ID_LEN							32
#define	FTM_GWID_LEN						FTM_ID_LEN
#define	FTM_DID_LEN							FTM_ID_LEN
#define	FTM_EPID_LEN						FTM_ID_LEN

typedef	FTM_CHAR	FTM_ID[FTM_ID_LEN+1];
typedef	FTM_CHAR	FTM_DID[FTM_ID_LEN+1];
typedef	FTM_CHAR	FTM_EPID[FTM_ID_LEN+1];

typedef	FTM_ID _PTR_	FTM_ID_PTR;
typedef	FTM_DID _PTR_	FTM_DID_PTR;
typedef	FTM_EPID _PTR_	FTM_EPID_PTR;

#define	FTM_HOST_LEN						256

#define	FTM_USER_ID_LEN						256
#define	FTM_PASSWD_LEN						256

#define	FTM_URL_LEN							256
#define	FTM_LOCATION_LEN					256
#define	FTM_SNMP_COMMUNITY_LEN				128
#define	FTM_SNMP_MIB_LEN					128

#define	FTM_MQTT_TOPIC_LEN					128

#define	FTM_DEVICE_NAME_LEN					256

#define	FTM_NAME_LEN						256
#define	FTM_UNIT_LEN						32

#define	FTM_INTERVAL_MIN					0
#define	FTM_INTERVAL_MAX					(60 * 60 * 24 * 1000)

#define	FTM_RET_OK							0x00000000
#define	FTM_RET_TRUE						0x00000000
#define	FTM_RET_ERROR						0x00000001
#define	FTM_RET_FALSE						0x00000001
#define	FTM_RET_INVALID_ARGUMENTS			0x00000002
#define	FTM_RET_DATA_LEN_RANGE				0x00000003	
#define	FTM_RET_OBJECT_NOT_FOUND			0x00000004
#define	FTM_RET_ALREADY_EXISTS				0x00000005
#define	FTM_RET_ALREADY_EXIST_OBJECT		0x00000005
#define	FTM_RET_NOT_EXISTS					0x00000006
#define	FTM_RET_FUNCTION_NOT_SUPPORTED		0x00000007
#define	FTM_RET_NOT_ENOUGH_MEMORY			0x00000008
#define	FTM_RET_ALREADY_INITIALIZED			0x00000009
#define	FTM_RET_CLIENT_HANDLE_INVALID		0x0000000A
#define	FTM_RET_NOT_INITIALIZED				0x0000000B
#define	FTM_RET_INVALID_COMMAND				0x0000000C
#define	FTM_RET_ALREADY_RUNNING				0x0000000D
#define	FTM_RET_INVALID_DATA				0x0000000E
#define	FTM_RET_ALREADY_STARTED				0x0000000F
#define	FTM_RET_NOT_START					0x00000010
#define	FTM_RET_NODE_IS_NOT_STOPPED			0x00000011

#define	FTM_RET_EP_IS_NOT_ATTACHED			0x00000012
#define	FTM_RET_EP_DID_NOT_START			0x00000013
#define	FTM_RET_TIMEOUT						0x00000014

#define	FTM_RET_THREAD_CREATION_ERROR		0x00000015
#define	FTM_RET_THREAD_JOIN_ERROR			0x00000016

#define	FTM_RET_NOT_COMPARABLE				0x00000017
#define	FTM_RET_LIST_EMPTY					0x00000018
#define	FTM_RET_LIST_NOT_INSERTABLE			0x00000019
#define	FTM_RET_BUFFER_TOO_SMALL			0x0000001A

#define	FTM_RET_INVALID_MESSAGE_TYPE		0x0000001B
#define	FTM_RET_OUT_OF_MEMORY				0x0000001C
#define	FTM_RET_PAYLOAD_IS_TOO_LARGE		0x0000001D
#define	FTM_RET_NOT_CONNECTED				0x0000001E

#define	FTM_RET_ALREADY_ATTACHED			0x0000001F
#define	FTM_RET_NOT_ATTACHED				0x00000020

#define	FTM_RET_INVALID_TYPE				0x00000021
#define	FTM_RET_INVALID_DID					0x00000022
#define	FTM_RET_INVALID_NAME				0x00000023
#define	FTM_RET_INVALID_LOCATION			0x00000024
#define	FTM_RET_INVALID_INTERVAL			0x00000025
#define	FTM_RET_INVALID_TIMEOUT				0x00000026
#define	FTM_RET_INVALID_UNIT				0x00000027	
#define	FTM_RET_INVALID_URL					0x00000028
#define	FTM_RET_INVALID_COMMUNITY			0x00000029	
#define	FTM_RET_INVALID_VERSION				0x0000002A	
#define	FTM_RET_INVALID_ID					0x0000002B

#define	FTM_RET_QUEUE_IS_FULL				0x0000002C
#define	FTM_RET_QUEUE_IS_EMPTY				0x0000002D
#define	FTM_RET_DATA_IS_TOO_LARGE			0x0000002E	

#define	FTM_RET_SM_IS_NOT_EXIST				0x0000002F
#define	FTM_RET_CANT_CREATE_SM				0x00000030
#define	FTM_RET_CANT_CREATE_SEMAPHORE		0x00000031
#define	FTM_RET_FAILED_TO_INIT_SEM			0x00000032

#define	FTM_RET_NOT_SUPPORTED_NODE_CLASS	0x00000033
#define	FTM_RET_NO_CLASS_INFO				0x00000034

#define	FTM_RET_OWNER_MISMATCH				0x00000035
#define	FTM_RET_INVALID_FORMAT				0x00000036

#define	FTM_RET_ASSERT						0x00000037
#define	FTM_RET_OBJECT_IS_DISABLED			0x00000038

#define	FTM_RET_RUNNING						0x00000039

#define	FTM_RET_INVALID_JSON_FORMAT			0x0000003A

#define	FTM_RET_FAILED_TO_READ_FILE			0x0000003B

#define	FTM_RET_DBIF_ERROR					0x00010000
#define	FTM_RET_DBIF_ALREADY_INITIALIZED	0x00010001

#define	FTM_RET_DBIF_DB_ERROR				0x00020000

#define	FTM_RET_COMM_ERROR					0x00030000
#define	FTM_RET_COMM_TIMEOUT					(FTM_RET_COMM_ERROR	| 1) 
#define	FTM_RET_COMM_ALREADY_CONNECTED			(FTM_RET_COMM_ERROR	| 2) 
#define	FTM_RET_COMM_INVALID_VALUE				(FTM_RET_COMM_ERROR	| 3) 
#define	FTM_RET_COMM_FAILED_TO_TRANSMIT_PACKETS	(FTM_RET_COMM_ERROR	| 4) 
#define	FTM_RET_COMM_SESSION_IS_FULL			(FTM_RET_COMM_ERROR	| 5) 
#define	FTM_RET_COMM_SOCK_ERROR					(FTM_RET_COMM_ERROR	| 6) 
#define	FTM_RET_COMM_PACKET_SEND_FAILED		(FTM_RET_COMM_ERROR	| 7) 
#define	FTM_RET_COMM_SOCKET_BIND_FAILED		(FTM_RET_COMM_ERROR	| 8) 
#define	FTM_RET_COMM_RECV_ERROR				(FTM_RET_COMM_ERROR	| 9) 
#define	FTM_RET_COMM_SEND_ERROR				(FTM_RET_COMM_ERROR	| 10) 
#define	FTM_RET_COMM_CTRL_ERROR				(FTM_RET_COMM_ERROR	| 11) 
#define	FTM_RET_COMM_CONNECTION_ERROR		(FTM_RET_COMM_ERROR | 12)
#define	FTM_RET_COMM_SOCKET_CLOSED			(FTM_RET_COMM_ERROR	| 13) 
#define	FTM_RET_COMM_ERRNO					(FTM_RET_COMM_ERROR | 0x00001000)


#define	FTM_RET_CONFIG_LOAD_ERROR			0X00040000
#define	FTM_RET_CONFIG_LOAD_FAILED			(FTM_RET_CONFIG_LOAD_ERROR | 1)
#define	FTM_RET_CONFIG_INVALID_OBJECT		(FTM_RET_CONFIG_LOAD_ERROR | 2)
#define	FTM_RET_CONFIG_BUFFER_TOO_SMALL		(FTM_RET_CONFIG_LOAD_ERROR | 3)
#define	FTM_RET_CONFIG_INVALID_VALUE		(FTM_RET_CONFIG_LOAD_ERROR | 4)

#define	FTM_RET_SNMP_ERROR					0x00050000
#define	FTM_RET_SNMP_INIT_FAILED			(FTM_RET_SNMP_ERROR | 1)
#define	FTM_RET_SNMP_TIMEOUT				(FTM_RET_SNMP_ERROR | 2)
#define	FTM_RET_SNMP_CANT_OPEN_SESSION		(FTM_RET_SNMP_ERROR | 3)
#define	FTM_RET_SNMP_CANT_FIND_OID			(FTM_RET_SNMP_ERROR | 4)
#define	FTM_RET_SNMP_NOT_SUPPORT_WRITE		(FTM_RET_SNMP_ERROR | 5)
#define	FTM_RET_SNMP_INVALID_MESSAGE_FORMAT	(FTM_RET_SNMP_ERROR | 6)

#define	FTM_RET_MQTT_ERROR					0x0006000
#define	FTM_RET_MQTT_INVALID_TOPIC			(FTM_RET_MQTT_ERROR | 1)
#define	FTM_RET_MQTT_INVALID_MESSAGE		(FTM_RET_MQTT_ERROR | 1)

#define	FTM_RET_SHELL_ERROR					0x0008000
#define	FTM_RET_SHELL_QUIT					(FTM_RET_SHELL_ERROR | 1)

#define	FTM_RET_MODBUS_ERROR				0x00090000

#define	FTM_RET_INTERNAL_ERROR				0x000F0000
#define	FTM_RET_CANT_CREATE_THREAD			(FTM_RET_INTERNAL_ERROR | 1)

FTM_RET	FTM_isValidDID(FTM_CHAR_PTR pDID);
FTM_RET	FTM_isValidName(FTM_CHAR_PTR pName);
FTM_RET	FTM_isValidLocation(FTM_CHAR_PTR pLocation);
FTM_RET	FTM_isValidUnit(FTM_CHAR_PTR pUnit);
FTM_RET	FTM_isValidURL(FTM_CHAR_PTR pUnit);
FTM_RET	FTM_isValidCommunity(FTM_CHAR_PTR pCommunity);
FTM_RET	FTM_isValidInterval(FTM_ULONG ulInterval);

#endif
