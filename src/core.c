#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "core.h"
#include "string.h"

struct lua_State* s_luaState;
char s_statusMsg[256] = {0};
char s_errorMsg[2048] = {0};
const char* s_errorPort = NULL;

struct lua_State* getLuaState(){ return s_luaState; }
char* getStatusMsg() { return s_statusMsg; }
char* getErrorMsg() { return s_errorMsg; }

void coreInit()
{
    s_luaState = NULL;
}

/* Start running the program @path = program_lua */
void coreStart(const char* program_lua, char* error_msg)
{
    if (s_luaState ) {
        lua_close(s_luaState);
    }
    s_luaState = luaL_newstate();
    lua_State* L = getLuaState();

    luaL_openlibs(L);
    /* Load the file containing the script we are going to run */
    int result = luaL_loadfile(L, program_lua);
    if (result) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        memset(error_msg, 0, strlen(error_msg));
        strcpy(error_msg, lua_tostring(L, -1));
    }
    else {
        result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (result) {
            memset(error_msg, 0, strlen(error_msg));
            strcpy(error_msg, lua_tostring(L, -1));
        }
    }

    if (result) {
        /* Something is wrong with the script */
        lua_pushstring(L, "Couldn't load file:");
        lua_setglobal(L, "g_statusMsg");
        lua_pushstring(L, error_msg);
        lua_setglobal(L, "g_errorMsg");
        return;
    }

    lua_pushnumber(L, 0.0);
    lua_setglobal(L, "g_time");
    lua_pushstring(L, "Luminos");
    lua_setglobal(L, "g_statusMsg");
}

void coreShutdown()
{
    lua_State* L = getLuaState();
    if (L != NULL) {
        lua_close(L);
    }
    s_luaState = NULL;
}

int coreExecPort(const char* port_name)
{
    lua_State* L = getLuaState();
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

void coreUpdateGlobals(float time)
{
	lua_State* L = getLuaState();
	lua_pushnumber(L, time);
	lua_setglobal(L, "g_time");
}
