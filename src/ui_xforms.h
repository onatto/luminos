#pragma once

#include <stdint.h>

struct SDL_KeyboardEvent;

#define FFI_EXPORT __declspec(dllexport)

int ui_init();
int ui_uploadMouseGlobals(void*);
int ui_debugPrintfStack(int base_y);
int ui_setNVGContext(void* ctx);

// For the FFI, export these as C functions
extern "C"
{
    FFI_EXPORT void ui_dbgTextPrintf(int y, const char* str);
	FFI_EXPORT void ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
	FFI_EXPORT void ui_drawPort(float x, float y, int widget_state, char r, char g, char b, char a);
    FFI_EXPORT uint8_t ui_getKeyboardState(uint16_t key);
}

bool ui_frameStart();
void ui_frameEnd();

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
