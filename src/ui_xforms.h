#pragma once

#include <stdint.h>

struct SDL_KeyboardEvent;
struct SDL_Window;

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT
#endif

int ui_init();
int ui_initGlobals();
int ui_uploadMouseGlobals(void*);
int ui_debugPrintfStack(int base_y);
int ui_setNVGContext(void* ctx);

// For the FFI, export these as C functions
extern "C"
{
    FFI_EXPORT void ui_dbgTextPrintf(int y, const char* str);
    FFI_EXPORT void ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
    FFI_EXPORT void ui_drawPort(float x, float y, int widget_state, char r, char g, char b, char a);
    FFI_EXPORT void ui_drawWire(float px, float py, float qx, float qy, int start_state, int end_state);
    FFI_EXPORT uint8_t ui_getKeyboardState(uint16_t key);
    FFI_EXPORT void ui_warpMouseInWindow(int x, int y);
    FFI_EXPORT void ui_saveNVGState();
    FFI_EXPORT void ui_restoreNVG();
    FFI_EXPORT void ui_setTextProperties(const char* font, float size, int align);
    FFI_EXPORT void ui_setTextColor(int r, int g, int b, int a);
    FFI_EXPORT void ui_drawText(float x, float y, const char* str);
}

bool ui_frameStart();
void ui_frameEnd();

union SDL_Event;
void ui_textInputEvent(SDL_Event* event);

struct KeyEvent
{
    enum Enum
    {
        NotPressed,
        Press,
        Release,
        Hold
    };
};

extern SDL_Window* sdl_wnd;
