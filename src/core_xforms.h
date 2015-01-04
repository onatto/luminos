#pragma once

int core_init();
int core_start(const char* program_lua, char* error_msg_out);
/* Ports are C-Lua interfaces, every port has different expectations from the Lua function it runs */
/* That is, different expectations for the inputs, different expectations for the outputs          */
int core_execPort(const char* port_name);
int core_shutdown();

int core_initGlobals();
int core_updateGlobals(float time);

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT
#endif

// For the FFI, export these as C functions
extern "C"
{
    FFI_EXPORT int cmd_restart(const char* filename);
}

/* States by the core module */
struct lua_State;

extern lua_State* s_luaState;
extern char s_statusMsg[256];
extern char s_errorMsg[2048];
extern const char* s_errorPort;

inline lua_State* get_luaState() { return s_luaState; }
inline char* get_statusMsg() { return s_statusMsg; }
inline char* get_errorMsg() { return s_errorMsg; }
