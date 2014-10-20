#include "ui_xforms.h"
#include "lua_xforms.h"
#include "lua.hpp"
#include "lauxlib.h"

#include "bgfx.h"
#include "entry/entry.h"

#include "bx/string.h"

#define TABLE_ENTRIES 6

int ui_init(int env)
{
    lua_State* L = get_luastate(env);
    lua_createtable(L, 0, TABLE_ENTRIES);
    lua_pushnumber(L, 0.0);
    lua_setfield(L, -2, "mx");
    lua_pushnumber(L, 0.0);
    lua_setfield(L, -2, "my");
    lua_pushnumber(L, 0.0);
    lua_setfield(L, -2, "mz");

    lua_pushboolean(L, false);
    lua_setfield(L, -2, "left");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "middle");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "right");
    lua_setglobal(L, "g_mouseState");
    return 0;
}

int ui_uploadMouseGlobals(void* _state, int env)
{
    const entry::MouseState* state = (const entry::MouseState*)_state;

    lua_State* L = get_luastate(env);
    lua_getglobal(L, "g_mouseState");
    lua_pushnumber(L, state->m_mx);
    lua_setfield(L, -2, "mx");
    lua_pushnumber(L, state->m_my);
    lua_setfield(L, -2, "my");
    lua_pushnumber(L, state->m_mz);
    lua_setfield(L, -2, "mz");

    lua_pushboolean(L, state->m_buttons[entry::MouseButton::Left]);
    lua_setfield(L, -2, "left");
    lua_pushboolean(L, state->m_buttons[entry::MouseButton::Middle]);
    lua_setfield(L, -2, "middle");
    lua_pushboolean(L, state->m_buttons[entry::MouseButton::Right]);
    lua_setfield(L, -2, "right");

    lua_pop(L, 1);
    return 0;
}

static char dump_str[256] = { 0 };
int ui_debugPrintfStack(int base, int env)
{
    static const int flt_size = 16;
    char flt[flt_size];
    lua_State* L = get_luastate(env);

    bgfx::dbgTextPrintf(0, base, 0x4f, "Stack info:");
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++)
    {
        memset(dump_str, 0, 256);
        int t = lua_type(L, i);
        strcat(dump_str, lua_typename(L, t));
        strcat(dump_str, " : ");
        switch (t)
        {
            case LUA_TSTRING:  /* strings */
                strcat(dump_str, lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:  /* booleans */
                strcat(dump_str, lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:  /* numbers */
                bx::snprintf(flt, 16, "%g", lua_tonumber(L, i));
                strcat(dump_str, flt);
                break;
            default:  /* other values */
                break;
        }
        bgfx::dbgTextPrintf(0, i + base, 0x4f, dump_str);
    }
    return top;
}
