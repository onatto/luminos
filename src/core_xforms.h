#pragma once

struct lua_State;

extern lua_State* s_luaState;

inline lua_State* get_luastate() { return s_luaState; }
int core_init();
int core_compileLua(const char* filename, char* error_msg_out);
int core_execPort(const char* port_name, char* error_msg_out);
int core_shutdown();

int core_initGlobals();
int core_updateGlobals(float time);

void cmd_compile(const char* filename, char* status_msg_out, char* error_msg_out);

/* Ports are C-Lua interfaces, every port has different expectations from the Lua function it runs */
/* That is, different expectations for the inputs, different expectations for the outputs          */
int port_programStart(const char* port_name, char* std_out);
