#pragma once

struct SDL_Window;
struct SDL_GLContext;

void wnd_resizeWindow(uint32 width, uint32 height);
SDL_Window* wnd_initSDL(uint32_t width, uint32_t height);
SDL_GLContext wnd_initGL(SDL_Window* wnd);
