#ifndef	__SHELL_CMD_H__
#define	__SHELL_CMD_H__

typedef enum
{
	FTDM_SHELL_CMD_UNKNOWN = 0,
	FTDM_SHELL_CMD_DAEMON,
	FTDM_SHELL_CMD_USAGE,
	FTDM_SHELL_CMD_VERSION 
}	FTDM_SHELL_CMD;

FTM_RET FTDM_shellCmdVersion(FTM_CHAR_PTR pProgramName);
FTM_RET FTDM_shellCmdUsage(FTM_CHAR_PTR pProgramName);
FTM_RET FTDM_shellCmdStartDaemon(void);

#endif
