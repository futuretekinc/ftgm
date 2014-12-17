#ifndef	__FTGM_TYPE_H__
#define	__FTGM_TYPE_H__

typedef	unsigned char	FTGM_DEVICE_ID[32];
typedef	unsigned char	FTGM_EP_ID[4];
typedef	FTGM_EP_ID	*	FTGM_EP_ID_PTR;
typedef	char			FTGM_DEVICE_URL[256];
typedef	char			FTGM_DEVICE_LOC[256];
typedef	char *			FTGM_STRING;
typedef	unsigned long	FTGM_ULONG, * FTGM_ULONG_PTR;
typedef	int				FTGM_INT, * FTGM_INT_PTR;
typedef	int				FTGM_BOOL, * FTGM_BOOL_PTR;
#define	FTGM_BOOL_FALSE	0
#define	FTGM_BOOL_TRUE	(!FTGM_BOOL_FALSE)

typedef	int				FTGM_RET;
#define	FTGM_RET_OK								0x00000000
#define	FTGM_RET_ERROR							0x00010000
#define	FTGM_RET_ERROR_ALREADY_BEEN_COMPLETED	(FTGM_RET_ERROR | 0x01) 
#define	FTGM_RET_ERROR_DB						0x00020000

typedef	unsigned long		FTGM_DEVICE_TYPE, *	FTGM_DEVICE_TYPE_PTR;

#define FTGM_DEVICE_UNKNOWN			0x00000000
#define	FTGM_DEVICE_FTM_50S 	 	0x01010001
#define	FTGM_DEVICE_FTM_100S 	 	0x01020001
#define	FTGM_DEVICE_FTE_E_SERIES 	0x02010001
#define	FTGM_DEVICE_FTE_W_SERIES 	0x02020001

typedef	struct 
{
	FTGM_DEVICE_ID		xDID;
	FTGM_DEVICE_TYPE	xType;
	FTGM_DEVICE_URL		xURL;
	FTGM_DEVICE_LOC		xLocation;	
}	FTGM_DEVICE_INFO, * FTGM_DEVICE_INFO_PTR;

typedef	unsigned long	FTGM_EP_TYPE, * FTGM_EP_TYPE_PTR;

typedef	struct
{
	FTGM_EP_ID			xEPID;
	FTGM_DEVICE_ID		xDID;
	FTGM_EP_TYPE		xType;
	FTGM_STRING			strName;
	FTGM_STRING			strUnit;
	FTGM_ULONG			nInterval;
	FTGM_DEVICE_ID		xParent;
}	FTGM_EP_INFO, * FTGM_EP_INFO_PTR;

typedef	struct
{
	FTGM_EP_ID			xEPID;
	FTGM_ULONG			nTime;
	FTGM_ULONG			nValue;
}	FTGM_EP_DATA, * FTGM_EP_DATA_PTR;

#endif
