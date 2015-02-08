#include "lua.h"
#include "lauxlib.h"

#include "core_xforms.h"
#include "string.h"

struct lua_State* s_luaState;
char s_statusMsg[256] = {0};
char s_errorMsg[2048] = {0};
const char* s_errorPort = NULL;

int core_init()
{
    s_luaState = NULL;
    return 0;
}

struct lua_State* get_luaState(){ return s_luaState; }
char* get_statusMsg() { return s_statusMsg; }
char* get_errorMsg() { return s_errorMsg; }

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

int core_execPort(const char* port_name)
{
    lua_State* L = get_luaState();
    // Top index before the port function call - so the function can return multiple variables
    int top = lua_gettop(L);
    lua_getglobal(L, "portDisplayRuntimeError");
    lua_getglobal(L, port_name);
    if (!lua_isfunction(L, -1))
        return -1;

    /* Ask Lua to run the global function with name 'port_name' */
    /* portDisplayRuntimeError is the error handler, see lua_pcall doc for details */
    int result = lua_pcall(L, 0, LUA_MULTRET, -2);
    if (result) {
        s_errorPort = port_name;
        return -result;
    }
    else {
        s_errorPort = NULL;
    }

    int nresults = lua_gettop(L) - top;
    return nresults;
}

int cmd_restart(const char* filename)
{
    int fail = core_start(filename, s_errorMsg);
	lua_State* L = get_luaState();
    if (fail)
    {
        lua_pushstring(L, "Couldn't load file:");
        lua_setglobal(L, "g_statusMsg");
        lua_pushstring(L, s_errorMsg);
        lua_setglobal(L, "g_errorMsg");
        return -1;
    }

    lua_pushnumber(L, 0.0);
    lua_setglobal(L, "g_time");
    lua_pushstring(L, "Luminos");
    lua_setglobal(L, "g_statusMsg");
    lua_pushstring(L, "think xform");
    return 0;
}

int core_updateGlobals(float time)
{
	lua_State* L = get_luaState();
	lua_pushnumber(L, time);
	lua_setglobal(L, "g_time");
	return 0;
}
