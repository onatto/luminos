#include "lua.hpp"
#include "lauxlib.h"

#include "lua_xforms.h"
#include "string.h"
#include <bx/string.h>

lua_State* states[LuaEnvironments::Count];

int initLua(int env)
{
    lua_State** L = &states[env];
    *L = luaL_newstate();
    luaL_openlibs(*L);
    return 0;
}

int compileLua(const char* filename, char* error_msg, int env)
{
    lua_State* L = get_luastate(env);
    /* Load the file containing the script we are going to run */
    int result = luaL_loadfile(L, filename);
    if (result) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }

    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }

    return result;
}

int shutdownLua(int env)
{
    lua_State* L = get_luastate(env);
    lua_close(L);
    return 0;
}

// xform is: for each different xform groups, the C side needs different interfaces
// like, the input handling port for the editor and tool rendering port for the editor
// in the OOP world, such seperate transforms are grouped together in a world model, which
// only wrap around the actual transformations(which are themselves are consisted of regular fine grain function calls and data generation
// with actual code)
// Staying close to hardware does not only mean C(faster readable assembly code - only the compiler expands it and optimizes it), but another optimizing software(by analysing the transformations of the program)
// optimizing for the data flow too, since the future is all about the data

// A port is a function that manages the
int execPort(const char* port_name, char* error_msg, int env)
{
    lua_State* L = get_luastate(env);

    // Top before the port function call - so the function can return multiple variables
    int top = lua_gettop(L);
    lua_getglobal(L, port_name);
    if (!lua_isfunction(L, -1))
        return -1;

    /* Ask Lua to run our little script */
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        strcpy(error_msg, lua_tostring(L, -1));
        return result;
    }

    int nresults = lua_gettop(L) - top;

    return nresults;
}

int port_programStart(const char* port_name, char* std_out, int env)
{
    lua_State* L = get_luastate(env);
    int top = lua_gettop(L);
    int numOutputs = execPort(port_name, std_out);
    if (numOutputs > 0)
    {
        size_t len = 0;
        if (!lua_isstring(L, -1))
            return -1;
        char* str = (char*)lua_tolstring(L, -1, &len);
        strncpy(std_out, str, len);
    }
    lua_settop(L, top);
    return 0;
}
// Meh, seems like a bad idea - doesn't get better than data(just write a xform(for reuse) for the logic that causes the event to happen)
// Events that happen per frame - ordered, unbuffered (one can buffer the critical events for the logic of the program anyway)
// int eventSink(const char* event_name, const char* err_msg, int env)

// For GUI input processing: GUI items have AABB references that get uploaded to a common AABB list that gets checked for events every frame
// So, a lot more optimizations are possible to reduce the number of AABB tests(JIT compiled hopefully)
// Gather events to GUI in a very specific transform
