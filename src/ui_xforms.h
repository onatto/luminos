#pragma once

int ui_init(int env = 0);
int ui_uploadMouseGlobals(void*, int env = 0);
int ui_debugPrintfStack(int base, int env = 0);
int ui_setNVGContext(void* ctx);

// For the FFI
extern "C"
{
	__declspec(dllexport) int ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
}
