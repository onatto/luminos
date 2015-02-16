#include <string.h>

#include "ui.h"
#include "core.h"
#include "gfx.h"

#include "network.h"

#include "lua.h"
#include "lauxlib.h"

#include "gl44.h"
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "blendish.h"

#define TABLE_ENTRIES 6
static NVGcontext* nvg;
static NVGcontext* nvg_blur;

#include "SDL2/SDL.h"

static Uint8 keyboard_state_prev[512];
static const Uint8 *keyboard_state;

static uint32 mouse_state;
static uint32 mouse_state_prev;
SDL_Window* sdl_wnd;

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"


#define BLUR_FBO_WIDTH 1920
#define BLUR_FBO_HEIGHT 1080

struct BlurFBO {
    uint16 width;
    uint16 height;
    uint32 fbo;
    uint32 color;
    uint32 depth;
};

static void initBlurFBO(struct BlurFBO* blur, uint16 w, uint16 h)
{
    blur->width = w; blur->height = h;
    blur->fbo = gfxCreateFramebuffer(w, h, TEX_RGBA8, TEX_D24F, &blur->color, &blur->depth);
}

struct UIData
{
    int fontHeader;
    int fontHeaderBold;
    struct BlurFBO blur;
};

static struct UIData data;
int uiInit()
{
    nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
    bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png", 0));

    data.fontHeader = nvgCreateFont(nvg, "header", "font/opensans.ttf");
    data.fontHeaderBold = nvgCreateFont(nvg, "header-bold", "font/opensans-bold.ttf");
    nvg_blur = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    nvgCreateFont(nvg_blur, "header", "font/opensans.ttf");
    nvgCreateFont(nvg_blur, "header-bold", "font/opensans-bold.ttf");
    initBlurFBO(&data.blur, BLUR_FBO_WIDTH, BLUR_FBO_HEIGHT);
    uiInitGlobals();
    return 0;
}

int uiInitGlobals()
{
    lua_State* L = getLuaState();
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

static int mx,my; // Mouse X,Y

int uiFrameStart(uint32 width, uint32 height)
{
    nvgBeginFrame(nvg, width, height, 1.f);

    uint32 mleft, mright, mmiddle, mmask;

    lua_State* L = getLuaState();
    keyboard_state = SDL_GetKeyboardState(NULL);
    mouse_state = SDL_GetMouseState(&mx, &my);

    mmask = SDL_BUTTON(SDL_BUTTON_LEFT);
    mleft = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);
    mmask = SDL_BUTTON(SDL_BUTTON_RIGHT);
    mright = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);
    mmask = SDL_BUTTON(SDL_BUTTON_MIDDLE);
    mmiddle = ((mouse_state_prev & mmask ) ? 0x2 : 0x0) | ((mouse_state & mmask) ? 0x1 : 0x0);

    if (uiGetKeyboardState(SDL_SCANCODE_LCTRL) == KeyEvent_Hold && uiGetKeyboardState(SDL_SCANCODE_Q) == KeyEvent_Press)
        return 1;

    if (uiGetKeyboardState(SDL_SCANCODE_F4) == KeyEvent_Press)
    {
        printf("%s", getErrorMsg());
    }

    if (uiGetKeyboardState(SDL_SCANCODE_F5) == KeyEvent_Press)
    {
        coreShutdown();
        coreStart("scripts/program.lua", getErrorMsg());
        uiInitGlobals();
        networkSetLua(getLuaState());
        coreExecPort("portProgramInit");
        return false;
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


    return 0;
}

void uiFrameEnd()
{
    memcpy((void*)keyboard_state_prev, keyboard_state, 512);
    mouse_state_prev = mouse_state;
    nvgEndFrame(nvg);
}

void uiResize(uint32 width, uint32 height)
{
    gfxResizeTexture(data.blur.color, TEX_RGBA8, width, height);
    gfxResizeTexture(data.blur.depth, TEX_D24F, width, height);
}
void uiRenderBlur(uint32 width, uint32 height)
{
    gfxBindFramebuffer(data.blur.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    nvgBeginFrame(nvg_blur, width, height, 1.f);
    coreExecPort("portRenderNodes");
    nvgEndFrame(nvg_blur);

    gfxBindFramebuffer(0);
    gfxBlitTexture(data.blur.color, 0.f, 0.f, (float)width, (float)height, (float)width, (float)height);
}

static inline bool AABBPointTest(float x, float y, float w, float h, float px, float py)
{
    if (x < px && y < py && px < x+w && py < y+h)
        return true;
    return false;
}

void uiDrawNode(float x, float y, float w, float h, int widgetState, const char* title, char r, char g, char b, char a)
{
    bool mouseOverNode = AABBPointTest(x, y, w, h, (float)mx, (float)my);
    // Outline
    nvgBeginPath(nvg_blur);
    nvgRoundedRect(nvg_blur, x, y, w, h, 10.f);
    nvgStrokeColor(nvg_blur, nvgRGBA(255, 0, 0, mouseOverNode ? 255 : 200));
    nvgStrokeWidth(nvg_blur, 1.2f);
    nvgStroke(nvg_blur);
    // Text
    nvgFillColor(nvg_blur, nvgRGBA(255, 0, 0,150));
    nvgFontFace(nvg_blur, "header");
    nvgFontSize(nvg_blur, 20.f);
    nvgTextAlign(nvg_blur, NVG_ALIGN_CENTER);
    nvgText(nvg_blur, x + w*0.5f, y + h*0.5f + 5.f, title, NULL);
}

uint8 uiGetKeyboardState(uint16 key)
{
    return (keyboard_state_prev[key] << 1) | (keyboard_state[key] << 0);
}

void uiDrawPort(float x, float y, int widgetState, char r, char g, char b, char a)
{
    bndNodePort(nvg, x, y, (BNDwidgetState)widgetState, nvgRGBA(r,g,b,a));
}

void uiDrawWire(float px, float py, float qx, float qy, int start_state, int end_state)
{
    bndNodeWire(nvg, px, py, qx, qy, (BNDwidgetState)start_state, (BNDwidgetState)end_state);
}
void uiWarpMouseInWindow(int x, int y)
{
    SDL_WarpMouseInWindow(sdl_wnd, x, y);
}

void uiSaveNVGState()
{
    nvgSave(nvg);
}

void uiRestoreNVGState()
{
    nvgRestore(nvg);
}
void uiSetTextProperties(const char* font, float size, int align)
{
    nvgFontFace(nvg, font);
    nvgFontSize(nvg, size);
    nvgTextAlign(nvg, align);
}
void uiSetTextColor(int r, int g, int b, int a)
{
	nvgFillColor(nvg, nvgRGBA(r,g,b,a));
}
void uiDrawText(float x, float y, const char* str)
{
    if (str) {
    nvgText(nvg, x, y, str, NULL);
    }
}

void uiTextInputEvent(SDL_Event* event)
{
    lua_State* L = getLuaState();
    lua_getglobal(L, "portDisplayRuntimeError");
    lua_getglobal(L, "portTextEdit");
    if (!lua_isfunction(L, -1))
        return;

    lua_pushlstring(L, (const char*)event->text.text, strlen(event->text.text));
    lua_pcall(L, 1, 0, -2);
}

void uiShutdown()
{
    nvgDeleteGL3(nvg);
    nvgDeleteGL3(nvg_blur);
}

void uiVisualiserFrame(float x, float y, float w, float h)
{
    nvgBeginPath(nvg);
    nvgStrokeColor(nvg, nvgRGBA(255, 0, 30, 140));
    nvgRect(nvg, x, y, w, h);
    nvgStroke(nvg);
}

void uiBlur(int toggle)
{
}
