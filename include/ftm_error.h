#ifndef	__FTM_ERROR_H__
#define	__FTM_ERROR_H__

#define	FTM_RET_OK							0x00000000
#define	FTM_RET_ERROR						0x00000001
#define	FTM_RET_INVALID_ARGUMENTS			0x00000002
#define	FTM_RET_DATA_LEN_RANGE				0x00000003	
#define	FTM_RET_OBJECT_NOT_FOUND			0x00000004
#define	FTM_RET_ALREADY_EXIST_OBJECT		0x00000005
#define	FTM_RET_FUNCTION_NOT_SUPPORTED		0x00000006
#define	FTM_RET_NOT_ENOUGH_MEMORY			0x00000007
#define	FTM_RET_ALREADY_INITIALIZED			0x00000008
#define	FTM_RET_CLIENT_HANDLE_INVALID		0x00000009
#define	FTM_RET_NOT_INITIALIZED				0x0000000A

#define	FTM_RET_DBIF_ERROR					0x00010000
#define	FTM_RET_DBIF_ALREADY_INITIALIZED	0x00010001

#define	FTM_RET_DBIF_DB_ERROR				0x00020000

#define	FTM_RET_COMM_ERROR					0x00030000
#define	FTM_RET_COMM_TIMEOUT				(FTM_RET_COMM_ERROR	| 1) 

#define	FTM_RET_CONFIG_LOAD_ERROR			0X00040000
#define	FTM_RET_CONFIG_LOAD_FAILED			(FTM_RET_CONFIG_LOAD_ERROR | 1)

#define	FTM_RET_SNMP_ERROR					0x00050000
#define	FTM_RET_SNMP_INIT_FAILED			(FTM_RET_SNMP_ERROR | 1)

#define	FTM_RET_INTERNAL_ERROR				0x000F0000
#define	FTM_RET_CANT_CREATE_THREAD			(FTM_RET_INTERNAL_ERROR | 1)

#endif
