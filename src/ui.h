#pragma once

#include <stdint.h>
#include <stdbool.h>

struct SDL_KeyboardEvent;
struct SDL_Window;

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

int uiInit();
int uiInitGlobals();
int uiUploadMouseGlobals(void*);
int uiDebugPrintfStack(int base_y);
int uiSetNVGContext(void* ctx);

// For the FFI, export these as C functions
FFI_EXPORT void uiDrawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
FFI_EXPORT void uiDrawPort(float x, float y, int widget_state, char r, char g, char b, char a);
FFI_EXPORT void uiDrawWire(float px, float py, float qx, float qy, int start_state, int end_state);
FFI_EXPORT uint8_t uiGetKeyboardState(uint16_t key);
FFI_EXPORT void uiWarpMouseInWindow(int x, int y);
FFI_EXPORT void uiSaveNVGState();
FFI_EXPORT void uiRestoreNVG();
FFI_EXPORT void uiSetTextProperties(const char* font, float size, int align);
FFI_EXPORT void uiSetTextColor(int r, int g, int b, int a);
FFI_EXPORT void uiDrawText(float x, float y, const char* str);

bool uiFrameStart();
void uiFrameEnd();

union SDL_Event;
void uiTextInputEvent(union SDL_Event* event);

enum KeyEvent
{
    KeyEvent_NotPressed,
    KeyEvent_Press,
    KeyEvent_Release,
    KeyEvent_Hold
};

extern struct SDL_Window* sdl_wnd;
