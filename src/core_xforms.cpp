#include "lua.hpp"
#include "lauxlib.h"

#include "core_xforms.h"
#include "string.h"

lua_State* s_luaState;

int core_init()
{
    s_luaState = luaL_newstate();
    luaL_openlibs(s_luaState);
    return 0;
}

int core_compileLua(const char* filename, char* error_msg)
{
    lua_State* L = get_luastate();
    /* Load the file containing the script we are going to run */
    int result = luaL_loadfile(L, filename);
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
    lua_State* L = get_luastate();
    lua_close(L);
    return 0;
}

int core_execPort(const char* port_name, char* error_msg)
{
    lua_State* L = get_luastate();

    // Top before the port function call - so the function can return multiple variables
    int top = lua_gettop(L);
    lua_getglobal(L, port_name);
    //if (!lua_isfunction(L, -1))
    //    return -1;

    /* Ask Lua to run our little script */
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }

    int nresults = lua_gettop(L) - top;

    return nresults;
}

int port_programStart(const char* port_name, char* std_out)
{
    lua_State* L = get_luastate();
    int top = lua_gettop(L);
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

void cmd_compile(const char* filename, char* status_msg, char* error_msg)
{
    int fail = core_compileLua(filename, error_msg);
    if (fail)
    {
		memset(status_msg, 0, strlen(status_msg));
        strcpy(status_msg, "Couldn't load file:");
    }
    else
    {
        memset(status_msg, 0, strlen(status_msg));
        strcpy(status_msg, "Compiled Lua successfully!");
        memset(error_msg, 0, strlen(error_msg));
        strcpy(error_msg, "Luminos");
    }
}

int core_initGlobals()
{
	lua_State* L = get_luastate();
	lua_pushnumber(L, 0.0);
	lua_setglobal(L, "g_time");
	return 0;
}

int core_updateGlobals(float time)
{
	lua_State* L = get_luastate();
	lua_pushnumber(L, time);
	lua_setglobal(L, "g_time");
	return 0;
}
