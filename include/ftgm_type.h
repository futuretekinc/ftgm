#ifndef	__FTGM_TYPE_H__
#define	__FTGM_TYPE_H__

typedef	unsigned char	FTGM_DID[32];
typedef	unsigned char	FTGM_EPID[4];

typedef	unsigned long		FTGM_DEVICE_TYPE;
typedef	FTGM_DEVICE_TYPE * 	FTGM_DEVICE_TYPE_PTR;

#define FTGM_DEVICE_UNKNOWN			0x00000000
#define	FTGM_DEVICE_FTM_50S 	 	0x01010001
#define	FTGM_DEVICE_FTM_100S 	 	0x01020001
#define	FTGM_DEVICE_FTE_E_SERIES 	0x02010001
#define	FTGM_DEVICE_FTE_W_SERIES 	0x02020001

typedef	unsigned long	FTGM_EP_TYPE;
#endif
