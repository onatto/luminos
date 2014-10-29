#pragma once

int core_init();
int core_compileLua(const char* filename, char* error_msg_out);
int core_execPort(const char* port_name, char* error_msg_out);
int core_shutdown();

int core_initGlobals();
int core_updateGlobals(float time);

/* Ports are C-Lua interfaces, every port has different expectations from the Lua function it runs */
/* That is, different expectations for the inputs, different expectations for the outputs          */
int port_programStart(const char* port_name, char* std_out);
int port_debugInfo(const char* port_name, const char* stack_trace_table, int x, int y);
int port_programInit(const char* port_name, char* error_msg);

#define FFI_EXPORT __declspec(dllexport)

// For the FFI, export these as C functions
extern "C"
{
    FFI_EXPORT void cmd_compile(const char* filename, char* status_msg_out, char* error_msg_out);
}

/* States by the core module */
struct lua_State;

extern lua_State* s_luaState;
extern char s_statusMsg[256];
extern char s_errorMsg[2048];

inline lua_State* get_luaState() { return s_luaState; }
inline char* get_statusMsg() { return s_statusMsg; }
inline char* get_errorMsg() { return s_errorMsg; }
