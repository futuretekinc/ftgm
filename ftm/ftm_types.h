#ifndef	__FTM_TYPES_H__
#define	__FTM_TYPES_H__

#include <stdint.h>

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

#define	FTM_FALSE		0
#define	FTM_TRUE		(!FTM_FALSE)

typedef	unsigned long	FTM_IP_ADDR, _PTR_ FTM_IP_ADDR_PTR;

typedef	unsigned long	FTM_RET, _PTR_ FTM_RET_PTR;
typedef	unsigned long	FTM_CMD, _PTR_ FTM_CMD_PTR;


#define	FTM_FILE_NAME_LEN	256
#endif
