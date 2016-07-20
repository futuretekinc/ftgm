#include <ctype.h>
#include <stdlib.h>    
#define	__USE_XOPEN
#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftom_client.h"

#define		FTOM_CLIENT_MAX_LINE	2048
#define		FTOM_CLIENT_MAX_ARGS	16


FTM_RET	FTOM_CLIENT_CMD_client(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_trace(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_EP(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_NODE(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_EP(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_EP_DATA(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_debug(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_discovery(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTOM_CLIENT_CMD_quit(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);


FTM_SHELL_CMD	_cmds[] = 
{
	{	
		.pString	= "client",
		.function	= FTOM_CLIENT_CMD_client,
		.pShortHelp	= "Client configuration.",
		.pHelp		= "<cmd>\n"\
					  "\tClient configuration."\
					  "COMMANDS:\n"\
					  "\tserver\n"\
					  "\t    - Server information.\n"
	},
	{	
		.pString	= "node",
		.function	= FTOM_CLIENT_CMD_NODE,
		.pShortHelp = "Node management command set.",
		.pHelp		= "<cmd>\n"\
					  "\tNode management command set.\n"\
					  "COMMANDS:\n"\
					  "\tadd <DID> SNMP <URL> <COMMUNITY>\n"\
					  "\tdel <DID>\n"\
					  "\tinfo <DID>\n"\
					  "\tlist\n"\
					  "PARAMETERS:\n"\
					  "\tDID    Node Identifier Number.\n"\
					  "\t       Max 32 characters\n"\
					  "\tTYPE	Define node type\n"\
					  "\t       SNMP - SNMP supported node\n"\
					  "\t       MODBUS/TCP - MODBUS over TCP/IP supported node\n"\
					  "\t       MODBUS - MODBUS over Serial supported node\n"\
					  "\tURL    Node location\n"\
					  "\t       (Ex. : www.node.com or 192.168.1.100)\n"\
					  "\tCOMMUNITY SNMP communication community\n"\
					  "\t       (Ex. : public, private,...)"
	},
	{	
		.pString	= "ep",
		.function   = FTOM_CLIENT_CMD_EP,
		.pShortHelp = "EndPoint management command set.",
		.pHelp      = "<cmd> ...\n"\
					  "\tEndPoint management.\n"\
					  "COMMANDS:\n"\
					  "\tadd    <EPID> <DID> [-n <Name>] [-u <Unit>] [-i <Interval>] [-t <timeout>]\n"\
					  "\tdel    <EPID>\n"\
					  "\tlist\n"\
					  "\t<EPID> [[name <name>] | [unit <unit>] | [interval <interval>]]\n"\
					  "PARAMETERS:\n"\
					  "\tEPID   EndPoint ID. 8Digit Hexa Number\n"\
					  "\t       (Ex. : 010A0003)\n"\
					  "\tDID	Node ID.\n"\
					  "\tname	EndPoint name.\n"\
					  "\tunit   Unit of measurement, a definite magnitude of a physical quantity.\n"\
					  "\tinterval Measurement interval.\n"\
					  "\ttimeout The response latency.\n"

	},
	{	
		.pString	= "data",
		.function	= FTOM_CLIENT_CMD_EP_DATA,
		.pShortHelp	= "EndPoint management command set.",
		.pHelp		= "<COMMAND> ...\n"\
					  "\tEndPoint data management.\n"\
					  "COMMANDS:\n"\
					  "\tadd    EPID <Time> <Value>\n"\
					  "\tdel    -i EPID <Index> <Count>\n"\
					  "\tdel    -t EPID <Begin Time> [End Time | Count]\n"\
					  "\tcount  -i EPID <Index>\n"\
					  "\tcount  -t EPID <Begin Time> <End Time>\n"\
					  "\tget    -i EPID <Index> <Count>\n"\
					  "\tget    -t EPID <Begin Time> <End Time> <Count>\n"\
					  "OPTIONS:\n"\
					  "\t-i     Index mode\n"\
					  "\t-t     Time mode\n"\
					  "PARAMETERS:\n"\
					  "\tEPID   EndPoint ID. 8Digit Hexa Number\n"\
					  "\t       (Ex. : 010A0003)\n"\
					  "\tTime   Format => YYYY-MM-DD HH:MM:SS\n"\
					  "\t       (Ex. : 2015-01-05 13:11:15)\n"\
					  "\tIndex  Decimal Number\n"\
					  "\tCount  Element count"
		
	},
	{	
		.pString	= "debug",
		.function	= FTOM_CLIENT_CMD_debug,
		.pShortHelp = "Debugging.",
		.pHelp 		= "\n"\
					  "\tDebugging."
	},
	{	
		.pString	= "discovery",
		.function	= FTOM_CLIENT_CMD_discovery,
		.pShortHelp = "Search for node and EP.",
		.pHelp 		= "<cmd>\n"\
					  "\tSearch for node and EP.\n"\
					  "COMMANDS:\n"\
					  "\tstart [<COUNT>]\n"\
					  "\t    - Start discovery.\n"\
					  "\tstop\n"\
					  "\t    - Stop discovery.\n"\
					  "PARAMETERS:\n"\
					  "\tCOUNT   The retry count of message for discovery.\n"
	},
	{	
		.pString	= "quit",
		.function	= FTOM_CLIENT_CMD_quit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	},
	{	
		.pString	= "trace",
		.function	= FTOM_CLIENT_CMD_trace,
		.pShortHelp = "Trace configuration.",
		.pHelp 		= "<cmd>\n"\
					  "\tTrace configuration.\n"\
					  "COMMANDS:\n"\
					  "\tlevel [<LEVEL>]\n"\
					  "PARAMETERS:\n"\
					  "\tLEVEL   Trace level\n"\
					  "\t        (all, trace, debug, information, warning, error, fatal)\n"
	}
};

FTM_ULONG	ulCmds = sizeof(_cmds) / sizeof(FTM_SHELL_CMD);


