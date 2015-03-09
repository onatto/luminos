#pragma once

#include <stdint.h>
#include "types.h"

struct SDL_KeyboardEvent;
struct SDL_Window;

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

int uiInit();
void uiShutdown();

// For the FFI, export these as C functions
FFI_EXPORT uint32 uiDrawNode(float x, float y, float w, float h, uint8 state, const char* title, uint8 numInputs, uint8 numOutputs);
FFI_EXPORT void uiDrawPort(float x, float y, char r, char g, char b, char a);
FFI_EXPORT void uiDrawWire(float px, float py, float qx, float qy);
FFI_EXPORT uint8 uiGetKeyboardState(uint16 key);
FFI_EXPORT void uiWarpMouseInWindow(int x, int y);
FFI_EXPORT void uiSaveNVGState();
FFI_EXPORT void uiRestoreNVG();
FFI_EXPORT void uiSetTextProperties(const char* font, float size, int align);
FFI_EXPORT void uiSetTextColor(int r, int g, int b, int a);
FFI_EXPORT void uiDrawText(float x, float y, const char* str);
FFI_EXPORT void uiVisualiserFrame(float x, float y, float w, float h);
FFI_EXPORT int32_t uiDrawNodeListFrame(float x, float y, float w, uint16_t numElems);
FFI_EXPORT int32_t uiDrawNodeListElement(float x, float y, float w, uint16_t elem, const char* str);

int uiFrameStart(uint32 width, uint32 height, float time);
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
void uiRenderBlur(uint32 width, uint32 height);
void uiResize(uint32 width, uint32 height);
