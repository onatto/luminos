#include "ui_xforms.h"
#include "core_xforms.h"
#include "lua.hpp"
#include "lauxlib.h"

#include "bgfx.h"

#include "bx/string.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"

#include "nanovg/nanovg.h"
#include "blendish.h"

#define TABLE_ENTRIES 6
static NVGcontext* nvg;

#include "SDL2/SDL.h"

static Uint8 keyboard_state_prev[512];
static const Uint8 *keyboard_state;

static uint32_t mouse_state;
static uint32_t mouse_state_prev;

int ui_init()
{
    lua_State* L = get_luaState();
    lua_createtable(L, 0, TABLE_ENTRIES);
    lua_pushnumber(L, 0.0);
    lua_setfield(L, -2, "mx");
    lua_pushnumber(L, 0.0);
    lua_setfield(L, -2, "my");

    lua_pushboolean(L, false);
    lua_setfield(L, -2, "left");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "middle");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "right");
    lua_setglobal(L, "g_mouseState");

    return 0;
}

bool ui_frameStart()
{
    int mx,my;
    uint32_t mleft, mright, mmiddle, mmask;

    lua_State* L = get_luaState();
    keyboard_state = SDL_GetKeyboardState(NULL);
    mouse_state = SDL_GetMouseState(&mx, &my);

    mmask = SDL_BUTTON(SDL_BUTTON_LEFT);
    mleft = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);
    mmask = SDL_BUTTON(SDL_BUTTON_RIGHT);
    mright = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);
    mmask = SDL_BUTTON(SDL_BUTTON_MIDDLE);
    mmiddle = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);

    // Use debug font to print information about this example.
    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 1, 0x4f, s_statusMsg);
    bgfx::dbgTextPrintf(0, 2, 0x6f, s_errorMsg);

    if (ui_getKeyboardState(SDL_SCANCODE_ESCAPE) == KeyEvent::Press)
        return true;

    if (ui_getKeyboardState(SDL_SCANCODE_F5) == KeyEvent::Press)
    {
        cmd_compile("scripts/program.lua", s_statusMsg, s_errorMsg);
    }

    lua_getglobal(L, "g_mouseState");
    lua_pushnumber(L, mx);
    lua_setfield(L, -2, "mx");
    lua_pushnumber(L, my);
    lua_setfield(L, -2, "my");

    lua_pushnumber(L,  mleft);
    lua_setfield(L, -2, "left");
    lua_pushnumber(L, mmiddle);
    lua_setfield(L, -2, "middle");
    lua_pushnumber(L, mright);
    lua_setfield(L, -2, "right");
    lua_pop(L, 1);

    return false;
}
void ui_frameEnd()
{
    memcpy((void*)keyboard_state_prev, keyboard_state, 512);
    mouse_state_prev = mouse_state;
}

static char dump_str[512] = { 0 };
int ui_debugPrintfStack(int base_y)
{
    static const int flt_size = 16;
    char flt[flt_size];
    lua_State* L = get_luaState();

    bgfx::dbgTextPrintf(0, base_y, 0x4f, "Stack info:");
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
        bgfx::dbgTextPrintf(0, i + base_y, 0x4f, dump_str);
    }
    return top;
}

int ui_setNVGContext(void* _nvg)
{
    nvg = (NVGcontext*)_nvg;
    return 0;
}

void ui_dbgTextPrintf(int y, const char* str)
{
    bgfx::dbgTextPrintf(0, y, 0x4f, str);
}

int ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a)
{
    bndNodeBackground(nvg, x, y, w, h, (BNDwidgetState)widget_state, BND_ICONID(5, 11), title, nvgRGBA(r, g, b, a));
    return 0;
}

uint8_t ui_getKeyboardState(uint16_t key)
{
    return (keyboard_state_prev[key] << 1) | (keyboard_state[key] << 0);
}

