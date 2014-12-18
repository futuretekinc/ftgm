#ifndef	__CMD_H__
#define	__CMD_H__

typedef enum
{
	FTDM_CMD_UNKNOWN = 0,
	FTDM_CMD_DAEMON,
	FTDM_CMD_USAGE,
	FTDM_CMD_VERSION 
}	FTDM_CMD;

void	FTDM_cmdVersion(FTGM_STRING strProgramName);
void	FTDM_cmdUsage(FTGM_STRING strProgramName);
void	FTDM_cmdStartDaemon(void);

#endif
