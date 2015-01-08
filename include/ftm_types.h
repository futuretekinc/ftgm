#ifndef	__FTM_TYPES_H__
#define	__FTM_TYPES_H__

#define	_PTR_	*

typedef	void			FTM_VOID, _PTR_ FTM_VOID_PTR;
typedef	char			FTM_CHAR, _PTR_ FTM_CHAR_PTR;
typedef	unsigned char	FTM_BYTE, _PTR_ FTM_BYTE_PTR;
typedef	int				FTM_INT, _PTR_ FTM_INT_PTR;
typedef	unsigned int	FTM_UINT, _PTR_ FTM_UINT_PTR;
typedef	unsigned short 	FTM_USHORT, _PTR_ FTM_USHORT_PTR;
typedef	long			FTM_LONG, _PTR_ FTM_LONG_PTR;
typedef	unsigned long	FTM_ULONG, _PTR_ FTM_ULONG_PTR;
typedef	int				FTM_BOOL, _PTR_ FTM_BOOL_PTR;
typedef	float			FTM_FLOAT, _PTR_ FTM_FLOAT_PTR;
typedef	double			FTM_DOUBLE, _PTR_ FTM_DOUBLE_PTR;

#define	FTM_BOOL_FALSE	0
#define	FTM_BOOL_TRUE	(!FTM_BOOL_FALSE)

typedef	unsigned long	FTM_IP_ADDR, _PTR_ FTM_IP_ADDR_PTR;

typedef	unsigned long	FTM_RET, _PTR_ FTM_RET_PTR;
typedef	unsigned long	FTM_CMD, _PTR_ FTM_CMD_PTR;


#define	FTM_FILE_NAME_LEN	256
#endif
