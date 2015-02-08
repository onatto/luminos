#pragma once

struct SDL_Window;

void wndResizeWindow(uint32 width, uint32 height);
struct SDL_Window* wndInitSDL(uint32_t width, uint32_t height);
void wndInitGL(struct SDL_Window* wnd);
