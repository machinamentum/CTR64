
#ifndef DEBUGGER_H
#define DEBUGGER_H

#define DEBUGGER_PORT 24537

struct dbg_command
{

};

struct dbg_info
{

};

#ifndef DEBUGGER_HOST
int DebuggerOpen();
//int DebuggerGetCommand(dbg_command *);
//int DebuggerSendInfo(dbg_info *);
int DebuggerClose();
int DebuggerPrint(const char *str);
#endif

#define DEBUGGER_CLIENT_STRING "ctrx_debugger"
#define DEBUGGER_HOST_STRING   "debugger_ctrx"

#endif
