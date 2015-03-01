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
// uint32 lastNodeID
// Used in DrawNode, counter assuming nodes are passed in the same order as last frame
// Set to 0 at ui_start
// prevNodeID is the node ID from previous frame
static uint32 lastNodeID;               
static uint32 prevNodeID;               
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
    uint32 blurv;
    uint32 blurh;
    uint32 pipe;
    uint32 blurvDst;
    uint32 blurhDst;
};

static void initBlurFBO(struct BlurFBO* blur, uint16 w, uint16 h)
{
    blur->width = w; blur->height = h;
    blur->fbo = gfxCreateFramebuffer(w, h, TEX_RGBA16F, TEX_D24F, &blur->color, &blur->depth);
    blur->blurv = gfxCreateShader("shaders/blur_v.cs", SHADER_COMP);
    blur->blurh = gfxCreateShader("shaders/blur_h.cs", SHADER_COMP);
    blur->pipe = gfxCreatePipeline();
    blur->blurvDst = gfxCreateImage2D(w, h, TEX_RGBA16F);
    blur->blurhDst = gfxCreateImage2D(w, h, TEX_RGBA16F);
}

struct UIData
{
    int fontHeader;
    int fontHeaderBold;
    struct BlurFBO blur;
};

static int uiInitGlobals()
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

    lua_pushnumber(L, 1920);
    lua_setglobal(L, "g_windowWidth");
    lua_pushnumber(L, 1080);
    lua_setglobal(L, "g_windowHeight");

    return 0;
}

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

static int mx,my; // Mouse X,Y
static float s_time;

int uiFrameStart(uint32 width, uint32 height, float time)
{
    nvgBeginFrame(nvg, width, height, 1.f);
    nvgBeginFrame(nvg_blur, width, height, 1.f);

    lastNodeID = 0;
    s_time = time;

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

    lua_pushnumber(L, width);
    lua_setglobal(L, "g_windowWidth");
    lua_pushnumber(L, height);
    lua_setglobal(L, "g_windowHeight");
    lua_pushnumber(L, time);
    lua_setglobal(L, "g_time");
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
    gfxResizeTexture(data.blur.color, TEX_RGBA16F, width, height);
    gfxResizeTexture(data.blur.depth, TEX_D24F, width, height);
    gfxResizeTexture(data.blur.blurvDst, TEX_RGBA16F, width, height);
    gfxResizeTexture(data.blur.blurhDst, TEX_RGBA16F, width, height);
}
void uiRenderBlur(uint32 width, uint32 height)
{
    /* Submit primitives to GPU after binding the framebuffer */
    gfxBindFramebuffer(data.blur.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    nvgEndFrame(nvg_blur);

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    // Vertical Blur
    gfxReplaceComputeShader(data.blur.pipe, data.blur.blurv);
    gfxBindPipeline(data.blur.pipe);
    gfxBindImage2D(data.blur.color,    0, TEX_RGBA16F);
    gfxBindImage2D(data.blur.blurvDst, 1, TEX_RGBA16F);
    glDispatchCompute(width/16, height/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Horizontal blur
    gfxReplaceComputeShader(data.blur.pipe, data.blur.blurh);
    gfxBindImage2D(data.blur.blurvDst, 0, TEX_RGBA16F);
    gfxBindImage2D(data.blur.blurhDst, 1, TEX_RGBA16F);
    glDispatchCompute(width/16, height/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    gfxBindFramebuffer(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    gfxBlitFramebuffer(data.blur.color, 0.f, 0.f, (float)width, (float)height, (float)width, (float)height);
    gfxBlitFramebuffer(data.blur.blurhDst, 0.f, 0.f, (float)width, (float)height, (float)width, (float)height);
}

static inline bool AABBPointTest(float x, float y, float w, float h, float px, float py)
{
    if (x < px && y < py && px < x+w && py < y+h)
        return true;
    return false;
}

//
enum NodeState {
    NODE_DEFAULT,
    NODE_HOVER,
    NODE_SELECTED,
};

static float timeSinceLastNode = 0.f;
static float s_zoom = 1.0;
uint32 uiDrawNode(float x, float y, float w, float h, uint8 state, const char* title, uint8 numInputs, uint8 numOutputs)
{
    float mouseX = (float)mx;
    float mouseY = (float)my;
    bool mouseOverNode = AABBPointTest(x, y, w, h, mouseX, mouseY);
    bool brighter = mouseOverNode || state == NODE_SELECTED;

    s_zoom = w / 180.f;

    lastNodeID++;
    if (mouseOverNode && lastNodeID != prevNodeID)
    {
       timeSinceLastNode = s_time;
       prevNodeID = lastNodeID;
    }
    // Outline
    nvgBeginPath(nvg_blur);
    nvgRoundedRect(nvg_blur, x, y, w, h, 4.f);
    nvgStrokeColor(nvg_blur, nvgRGBA(180, 0, 0, brighter ? 180 : 130));
    nvgStrokeWidth(nvg_blur, brighter ? 1.1f : 1.f);
    nvgStroke(nvg_blur);
    // Text
    nvgFillColor(nvg_blur, nvgRGBA(255, 0, 0,brighter ? 190 : 140));
    nvgFontFace(nvg_blur, "header");
    if (mouseOverNode) {
        const float scale = 0.25f;
        const float freq = 3.14f * 100.f / 60.f;
        float mult = w / 200.f * (1.0f + sin(freq * (s_time-timeSinceLastNode)) * scale );
        nvgFontSize(nvg_blur, 26.f * mult);
    }
    else {
        nvgFontSize(nvg_blur, 26.f * w / 200.0f);
    }
    nvgTextAlign(nvg_blur, NVG_ALIGN_CENTER);
    nvgText(nvg_blur, x + w*0.5f, y + h*0.5f + 5.f, title, NULL);

    // Ports
    uint8 mouseOverInput  = 0;
    uint8 mouseOverOutput = 0;
    float spacing = w / (float)(numInputs+1);
    float portY = y + h;
    for (uint8 i=1; i <= numInputs; i++)
    {
        float portX = x + spacing * i;
        bool mouseOver = AABBPointTest(portX - 8.f, portY - 8.f, 16.f, 16.f, mouseX, mouseY);
        mouseOverInput = mouseOver ? i : mouseOverInput;
        uiDrawPort(portX, portY, 0, 0, 100, 255, mouseOver ? 150 : 90);
    }

    spacing = w / (float)(numOutputs+1);
    portY = y;
    for (uint8 i=1; i <= numOutputs; i++)
    {
        float portX = x + spacing * i;
        bool mouseOver = AABBPointTest(portX - 8.f, portY - 8.f, 16.f, 16.f, mouseX, mouseY);
        mouseOverOutput = mouseOver ? i : mouseOverOutput;
        uiDrawPort(portX, portY, 0, 255, 0, 0, mouseOver ? 150 : 90);
    }

    return (mouseOverInput << 16) | (mouseOverOutput << 8)  | (mouseOverNode << 0);
}

uint8 uiGetKeyboardState(uint16 key)
{
    return (keyboard_state_prev[key] << 1) | (keyboard_state[key] << 0);
}

void uiDrawPort(float x, float y, int widgetState, char r, char g, char b, char a)
{
    nvgBeginPath(nvg_blur);
    nvgCircle(nvg_blur, x, y, 5.f);
    nvgStrokeColor(nvg_blur, bnd_theme.nodeTheme.wiresColor);
    nvgStrokeWidth(nvg_blur,1.0f * s_zoom);
    nvgStroke(nvg_blur);
    nvgFillColor(nvg_blur, nvgRGBA(r,g,b,a*s_zoom));
    nvgFill(nvg_blur);
}

void uiDrawWire(float px, float py, float qx, float qy, int start_state, int end_state)
{
    float tmp;
    if (py<qy) {
        tmp = px;
        px = qx;
        qx = tmp;
        tmp = py;
        py = qy;
        qy = tmp;
    }
    float length = fmax(abs(qy - py),abs(qx - px));
    float curving = 3.f; // between 0,10
    float delta = length * curving * 0.1f;
    nvgBeginPath(nvg_blur);
    nvgMoveTo(nvg_blur, px, py);
    nvgBezierTo(nvg_blur, 
        px + delta, py,
        qx - delta, qy,
        qx, qy);

    nvgStrokeColor(nvg_blur, nvgRGBA(200, 0, 0, 180));
    nvgStrokeWidth(nvg_blur, 0.9f * s_zoom);
    nvgStroke(nvg_blur);

    float time_int_part;
    float speed = 0.90f + sin((float)lastNodeID * 3.53f) * 0.5f;
    float tt = modf(s_time, &time_int_part);
    float time = tt*speed + (float)lastNodeID * 0.373f;
    //float t = modf(time, &time_int_part);
    float t = modf(s_time * speed, &time_int_part);
    float t_1 = (1.0f-t);
    // Coefficients for the Bezier
    float b0 = (t_1)*(t_1)*(t_1);
    float b1 = 3.f*(t_1)*(t_1)*t;
    float b2 = 3.f*(t_1)*t*t;
    float b3 = t*t*t;

    float x = b0*px + b1*(px+delta) + b2*(qx-delta) + b3*qx;
    float y = b0*py + b1*(py) + b2*(qy) + b3*qy;

    nvgBeginPath(nvg_blur);
    nvgCircle(nvg_blur, x, y, 2.f * s_zoom);
    nvgFillColor(nvg_blur, nvgRGBA(255,0,0,200));
    nvgFill(nvg_blur);
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
    nvgBeginPath(nvg_blur);
    nvgStrokeColor(nvg_blur, nvgRGBA(255, 0, 30, 140));
    nvgRect(nvg_blur, x, y, w, h);
    nvgStroke(nvg_blur);
}

void uiParseTransforms()
{

}
