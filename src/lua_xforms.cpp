#include "lua_xforms.h"
#include "string.h"

static lua_State* states[LuaEnvironments::Count];

inline lua_State** get_luastate(int env)
{
    return &states[env];
}
int initLua(int env)
{
    lua_State** L = get_luastate(env);
    *L = luaL_newstate();
    return 0;
}

int compileLua(const char* filename, char* error_msg, int env)
{
    lua_State* L = *get_luastate(env);
    /* Load the file containing the script we are going to run */
    int status = luaL_loadfile(L, filename);
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        strcpy(error_msg, lua_tostring(L, -1));
    }
    return status;
}

int shutdownLua(int env)
{
    lua_State* L = *get_luastate(env);
    lua_close(L);
    return 0;
}
