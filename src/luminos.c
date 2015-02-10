/*
 * Copyright 2014 Onat Turkcuoglu. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_X11
#undef SDL_VIDEO_DRIVER_WINDOWS
#undef main

#include "types.h"
#include "core.h"
#include "network.h"
#include "windowing.h"

#include "gl44.h"
#include "lua.h"
#include "lauxlib.h"

#include "ui.h"

#include "types.h"

static bool quit = false;
int main(int _argc, char** _argv)
{
    UNUSED(_argc);
    UNUSED(_argv);
    uint32_t width = 1920;
    uint32_t height = 1080;

    SDL_Window* wnd = wndInitSDL(width, height);
    wndInitGL(wnd);

    glClearColor(0.1,0.1,0.1,1);
    glClearStencil(0);
    glEnable(GL_STENCIL_FUNC);
    
    // Init gfx
    gfxInit();

    // Init core module
    coreInit();
    coreStart("scripts/program.lua", getErrorMsg());
    coreExecPort("portProgramInit");

    // Init UI module - depends on core
    uiInit();

    // Init network module - depends on core
    lua_getglobal(getLuaState(), "serverIP");
    const char* server_ip = lua_tolstring(getLuaState(), -1, NULL);
    networkInit(getLuaState(), server_ip, 3333);

    int64_t timeOffset = SDL_GetPerformanceCounter();
    SDL_Event event;
    while (!quit)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    width = event.window.data1;
                    height = event.window.data2;
                    wndResizeWindow(width, height);
                    break;
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
               uiTextInputEvent(&event); 
            }
        }

        int64 now = SDL_GetPerformanceCounter();
        const double freq = SDL_GetPerformanceFrequency();
        float time = (float)( (now-timeOffset)/freq);

        coreUpdateGlobals(time);

        quit |= uiFrameStart(width, height);
        if (!s_errorPort)
            coreExecPort("portProgramStart");
        else
            coreExecPort(s_errorPort);

        uiFrameEnd();
        networkUpdate();
        networkFlushWrites();
        SDL_WaitEventTimeout(NULL, 16);
	SDL_GL_SwapWindow(wnd);
    }

    uiShutdown();
    networkShutdown();
    coreShutdown();
    gfxShutdown();

    SDL_DestroyWindow(sdl_wnd);
    SDL_Quit();
    return 0;
}
