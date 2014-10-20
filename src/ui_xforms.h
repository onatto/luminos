#pragma once

struct SDL_KeyboardEvent;

#define FFI_EXPORT __declspec(dllexport)

int ui_init(int env = 0);
int ui_uploadMouseGlobals(void*, int env = 0);
int ui_debugPrintfStack(int base, int env = 0);
int ui_setNVGContext(void* ctx);

// For the FFI
extern "C"
{
    FFI_EXPORT void ui_dbgTextPrintf(int y, const char* str);
	FFI_EXPORT int ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
}

int ui_handleKeyEvent(const /*SDL_KeyboardEvent*/ void* key_ev);
int ui_handleMouseEvent(const /*SDL_MouseEvent*/ void* mouse_ev);