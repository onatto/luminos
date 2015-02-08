#pragma once

void coreInit();
void coreStart(const char* program_lua, char* error_msg_out);
/* Execute Lua function <port_name> */
int coreExecPort(const char* port_name);
void coreUpdateGlobals(float time);
void coreShutdown();

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

/* States by the core module */
struct lua_State;

extern struct lua_State* s_luaState;
extern char s_statusMsg[256];
extern char s_errorMsg[2048];
extern const char* s_errorPort;

struct lua_State* getLuaState();
char* getStatusMsg();
char* getErrorMsg();
