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
FTM_RET	FTOM_CLIENT_CMD_trigger(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
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
					  "\tadd        Add new node.\n"\
					  "\t           add <MODEL> <DID> [<opt> <opt> ...]\n"\
					  "\t           Ex.) add 00405CABCDEF -t SNMP -u 192.168.1.100 -c test\n"\
					  "\tdel        Remove the node\n"\
					  "\t           del <DID>\n"\
					  "\tinfo       Display node information\n"\
					  "\t           info <DID> <DID> ...\n"\
					  "\tlist       Display a list of node information.\n"\
					  "\tmodel      Display a list of supported node model.\n"\
					  "OPTONS:\n"\
					  "\t-t <TYPE>  Define node type\n"\
					  "\t-u <URL>   Internet address\n"\
					  "\t-c <COMM>  SNMP communication community\n"\
					  "\t-l <STRING>  Node location\n"\
					  "PARAMETERS:\n"\
					  "\tMODEL      Model of device.\n"\
					  "\t           Ex.)FTE-E, FTM-50S,...\n"\
					  "\tDID        Node Identifier Number.\n"\
					  "\t           Max 32 characters\n"\
					  "\tTYPE       Type of Node\n"\
					  "\t           SNMP - SNMP supported node\n"\
					  "\t           MBTCP - MODBUS over TCP/IP supported node\n"\
					  "\t           MB - MODBUS over Serial supported node\n"\
					  "\tURL        Internet IP or URL\n"\
					  "\t           Ex.) www.node.com or 192.168.1.100\n"\
					  "\tCOMM       SNMP communication community\n"\
					  "\t           Ex.) public, private,..."
	},
	{	
		.pString	= "ep",
		.function   = FTOM_CLIENT_CMD_EP,
		.pShortHelp = "EndPoint management command set.",
		.pHelp      = "<CMD>\n"\
					  "\tEndPoint management.\n"\
					  "COMMANDS:\n"\
					  "\tadd    Add new EP.\n"\
					  "\t       add <EPID> <DID> [-n <NAME>] [-u <UNIT>] [-i <MSECS>] [-t <MSECS>]\n"\
					  "\tdel    Remove the EP.\n"\
					  "\t       del <EPID>\n"\
					  "\tlist   Output a list of EPs.\n"\
					  "\tcount  Count of EPs.\n"\
					  "\tinfo   Output detail information of EP.\n"\
					  "\t       info <EPID> [<EPID> ...]\n"\
					  "\tstart  Activate EP\n"\
					  "\t       start <EPID> [<EPID> ...]\n"\
					  "\tstop   Deactivate EP\n"\
					  "\t       stop <EPID> [<EPID> ...]\n"\
					  "\tset    Set EP attributes.\n"
					  "\t       set <EPID> [-n <NAME>] [-U <UNIT>] [-i <MSECS>] [-t <MSECS>]\n"\
					  "\tdata   Output EP data\n"\
					  "\t       data <EPID> <SUBCMD> [<OPT> ...]\n"\
					  "OPTIONS:\n"\
					  "\t-m     Data operation mode\n"\
					  "\t       time - Time base mode\n"\
					  "\t       index - index base mode\n"\
					  "\t-i     Start index (Index mode only)\n"\
					  "\t-c     Maximum data count\n"\
					  "\t-s     Start time (Time mode only)\n"\
					  "\t-e     End time (Time mode only)\n"\
					  "PARAMETERS:\n"\
					  "\tEPID   EndPoint ID(8 digits Hexa Number).\n"\
					  "\t       (Ex. : 010A0003)\n"\
					  "\tDID	Node ID(Max 32 characters).\n"\
					  "\tNAME	EndPoint name.\n"\
					  "\tUNIT   Unit of measurement, a definite magnitude of a physical quantity.\n"\
					  "\tMSECS  Milliseconds.\n"\
					  "\tTIME   Format => YYYY-MM-DD HH:MM:SS\n"\
					  "\t       (Ex. : 2015-01-05 13:11:15)\n"\
					  "\tINDEX  Decimal Number\n"\
					  "\tCOUNT  Element count\n"\
					  "\tSBUCMD List of data management command\n"\
					  "\t       add - Add data\n"\
					  "\t       del - Remove data\n"\
					  "\t       get - Output data\n"

	},
	{	
		.pString	= "trigger",
		.function   = FTOM_CLIENT_CMD_trigger,
		.pShortHelp = "Trigger management command set.",
		.pHelp      = "<CMD>\n"\
					  "\tTrigger management.\n"\
					  "COMMANDS:\n"\
					  "\tadd     Add new Trigger.\n"\
					  "\t        add <ID> <EPID> -t <TYPE> [-n <NAME>] [-u <VALUE>] [-l <VALUE>] [-v <VALUE>]\n"\
					  "\tdel     Remove the trigger.\n"\
					  "\t        del <ID>\n"\
					  "\tlist    Output a list of trigger.\n"\
					  "\tcount   Count of trigger.\n"\
					  "\tinfo    Output detail information of trigger.\n"\
					  "\t        info <ID> [<ID> ...]\n"\
					  "\tenable  Activate trigger\n"\
					  "\t        enable <ID> [<ID> ...]\n"\
					  "\tdisable Deactivate trigger\n"\
					  "\t        disable <ID> [<ID> ...]\n"\
					  "\tset     Set trigger attributes.\n"
					  "\t        set <ID> [-n <NAME>] [-l <VALUE>] [-u <VALUE>] [-v <VALUE>]\n"\
					  "OPTIONS:\n"\
					  "\t-n      Trigger name.\n"\
					  "\t-t      Trigger type.\n"\
					  "\t        include - When included within a range.\n"\
					  "\t        except - When out of range.\n"\
					  "\t        above - Greate then the setting value.\n"\
					  "\t        below - Less then the setting value.\n"\
					  "\t        changed - The value change.\n"\
					  "\t-u      Trigger condition upper value(Condition : Include, Except).\n"\
					  "\t-l      Trigger condition lower value(Condition : Include, Except).\n"\
					  "\t-v      Trigger condition value(Condition : Above, Below).\n"\
					  "PARAMETERS:\n"\
					  "\tID      Trigger ID(Max 32 digits Hexa Number).\n"\
					  "\t        (Ex. : 010A0003)\n"\
					  "\tEPID	 EPID(Max 32 characters).\n"

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
		.pHelp 		= "<cmd> [<opt> <opt> ...]\n"\
					  "\tSearch for node and EP.\n"\
					  "COMMANDS:\n"\
					  "\tstart      Start discovery.\n"\
					  "\t           ex) start [-i <IP>] [-p <PORT>] [-c <COUNT>]\n"\
					  "\tlist       Display a list of discovered object.\n"\
					  "\t           ex) list [-u]\n"\
					  "OPTIONS:\n"\
					  "\t-i <IP>    Search network.\n"\
					  "\t-p <PORT>  Service port.\n"\
					  "\t-c <COUNT> Send message count.\n"\
					  "\t-u         Unregistered object only.\n"
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


