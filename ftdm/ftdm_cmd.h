#ifndef	__CMD_H__
#define	__CMD_H__

typedef enum
{
	FTDM_CMD_UNKNOWN = 0,
	FTDM_CMD_DAEMON,
	FTDM_CMD_USAGE,
	FTDM_CMD_VERSION 
}	FTDM_CMD;

FTGM_RET FTDM_cmdVersion(FTGM_STRING strProgramName);
FTGM_RET FTDM_cmdUsage(FTGM_STRING strProgramName);
FTGM_RET FTDM_cmdStartDaemon(void);

#endif
