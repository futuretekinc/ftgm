#ifndef	__FTM_ERROR_H__
#define	__FTM_ERROR_H__

#define	FTM_RET_OK							0x00000000
#define	FTM_RET_ERROR						0x00000001
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

#define	FTM_RET_DBIF_ERROR					0x00010000
#define	FTM_RET_DBIF_ALREADY_INITIALIZED	0x00010001

#define	FTM_RET_DBIF_DB_ERROR				0x00020000

#define	FTM_RET_COMM_ERROR					0x00030000
#define	FTM_RET_COMM_TIMEOUT				(FTM_RET_COMM_ERROR	| 1) 

#define	FTM_RET_CONFIG_LOAD_ERROR			0X00040000
#define	FTM_RET_CONFIG_LOAD_FAILED			(FTM_RET_CONFIG_LOAD_ERROR | 1)
#define	FTM_RET_CONFIG_INVALID_OBJECT		(FTM_RET_CONFIG_LOAD_ERROR | 2)

#define	FTM_RET_SNMP_ERROR					0x00050000
#define	FTM_RET_SNMP_INIT_FAILED			(FTM_RET_SNMP_ERROR | 1)
#define	FTM_RET_SNMP_TIMEOUT				(FTM_RET_SNMP_ERROR | 2)
#define	FTM_RET_SNMP_CANT_OPEN_SESSION		(FTM_RET_SNMP_ERROR | 3)
#define	FTM_RET_SNMP_CANT_FIND_OID			(FTM_RET_SNMP_ERROR | 4)

#define	FTM_RET_SHELL_ERROR					0x0006000
#define	FTM_RET_SHELL_QUIT					(FTM_RET_SHELL_ERROR | 1)

#define	FTM_RET_INTERNAL_ERROR				0x000F0000
#define	FTM_RET_CANT_CREATE_THREAD			(FTM_RET_INTERNAL_ERROR | 1)

#endif
