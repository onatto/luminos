#include "lua.hpp"
#include "lauxlib.h"

#include "core_xforms.h"
#include "string.h"

lua_State* s_luaState;
char s_statusMsg[256] = {0};
char s_errorMsg[2048] = {0};

int core_init()
{
    s_luaState = NULL;
    return 0;
}

int core_start(const char* program_lua, char* error_msg)
{
    // Every program == workspace is retained within it's own lua_state, the remaining task is to just
    // adjust a data exchange interface between different states and boom, you got modules inside the 
    // program
    // Combined with github, one can always keep in sync with own workspace
    // Oh, this was also in Smalltalk and node.js
    
    if (s_luaState ) {
        lua_close(s_luaState);
    }
    s_luaState = luaL_newstate();
    lua_State* L = get_luaState();

    luaL_openlibs(L);
    /* Load the file containing the script we are going to run */
    int result = luaL_loadfile(L, program_lua);
    if (result) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
		memset(error_msg, 0, strlen(error_msg));
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }

    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
		memset(error_msg, 0, strlen(error_msg));
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }
    return result;
}

int core_shutdown()
{
    lua_State* L = get_luaState();
    if (L != NULL) {
        lua_close(L);
    }
    s_luaState = NULL;
    return 0;
}

int core_execPort(const char* port_name, char* error_msg)
{
    lua_State* L = get_luaState();
// Top before the port function call - so the function can return multiple variables
    int top = lua_gettop(L);
    lua_getglobal(L, port_name);
    if (!lua_isfunction(L, -1))
        return -1;

    /* Ask Lua to run our little script */
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        return result;
    }

    int nresults = lua_gettop(L) - top;

    return nresults;
}

int port_programStart(const char* port_name, char* std_out)
{
    lua_State* L = get_luaState();
    int top = lua_gettop(L);
    memset(std_out, 0, strlen(std_out));
    int numOutputs = core_execPort(port_name, std_out);
    if (numOutputs > 0)
    {
        size_t len = 0;
        if (!lua_isstring(L, -1))
            return -1;
        char* str = (char*)lua_tolstring(L, -1, &len);
		memset(std_out, 0, strlen(std_out));
        strncpy(std_out, str, len);
    }
    lua_settop(L, top);
    return 0;
}

int port_programInit(const char* port_name, char* error_msg)
{
    lua_State* L = get_luaState();
    int top = lua_gettop(L);
    int numOutputs = core_execPort(port_name, error_msg);
    lua_settop(L, top);
    return 0;
}

void cmd_restart(const char* filename)
{
    int fail = core_start(filename, s_errorMsg);
	lua_State* L = get_luaState();
    if (fail)
    {
        lua_pushstring(L, "Couldn't load file:");
        lua_setglobal(L, "g_statusMsg");
        lua_pushstring(L, s_errorMsg);
        lua_setglobal(L, "g_errorMsg");
    }
    else
    {
        lua_pushnumber(L, 0.0);
        lua_setglobal(L, "g_time");
        lua_pushstring(L, "Luminos");
        lua_setglobal(L, "g_statusMsg");
        lua_pushstring(L, "think xform");
        lua_setglobal(L, "g_errorMsg");
    }
}

int core_updateGlobals(float time)
{
	lua_State* L = get_luaState();
	lua_pushnumber(L, time);
	lua_setglobal(L, "g_time");
	return 0;
}
