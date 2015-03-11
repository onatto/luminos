#include <stdio.h>
#include "types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "gl44.h"

void wndResizeWindow(uint32 width, uint32 height)
{
    glViewport(0,0,width,height);
}

struct SDL_Window* wndInitSDL(uint32_t width, uint32_t height)
{
    SDL_Window* wnd;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    wnd = SDL_CreateWindow("luminos"
                    , 0
                    , 0
                    , width
                    , height
                    , SDL_WINDOW_SHOWN
                    | SDL_WINDOW_RESIZABLE
                    | SDL_WINDOW_OPENGL
                    );

    if (wnd == NULL) {
	printf("Couldn't create SDL window. %s\n", SDL_GetError());
	return NULL;
    }

    return wnd;
}

void wndInitGL(struct SDL_Window* wnd)
{
    SDL_GL_CreateContext(wnd);
    if(ogl_LoadFunctions() == ogl_LOAD_FAILED) {
        printf("Loading GL functions failed.\n");
    }
}

void* wndHandle(struct SDL_Window* wnd)
{
    SDL_SysWMinfo info;
    SDL_GetWindowWMInfo(wnd, &info);
#ifdef MSVC
    return info.info.win.window;
#else
    return info.info.x11.window;
#endif
}
