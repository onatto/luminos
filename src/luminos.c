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

#include "nanovg/nanovg.h"

#include "windowing.h"
#include "gl_core_4_4.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

#include "lua.h"
#include "lauxlib.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"
#include "ui.h"


static bool quit = false;
int main(int _argc, char** _argv)
{
    uint32_t width = 1920;
    uint32_t height = 1080;

    SDL_Window* wnd = wndInitSDL(width, height);
    wndInitGL(wnd);

    NVGcontext* nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    uiSetNVGContext(nvg);

    bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
    bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png", 0));

    glClearColor(0.1,0.1,0.1,1);
    glClearStencil(0);
    glEnable(GL_STENCIL_FUNC);
    int64_t timeOffset = SDL_GetPerformanceCounter();

    // Setup Lua
    coreInit();

    coreStart("scripts/program.lua", getErrorMsg());
    uiInit();
    coreExecPort("portProgramInit");

    lua_getglobal(getLuaState(), "serverIP");
    const char* server_ip = lua_tolstring(getLuaState(), -1, NULL);

    networkInit(getLuaState(), server_ip, 3333);

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
        if (!quit) {
            quit = uiFrameStart();
        }

        int64 now = SDL_GetPerformanceCounter();
        const double freq = SDL_GetPerformanceFrequency();
        float time = (float)( (now-timeOffset)/freq);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        coreUpdateGlobals(time);

        nvgBeginFrame(nvg, width, height, 1.f);
        
        if (!s_errorPort)
            coreExecPort("portProgramStart");
        else
            coreExecPort(s_errorPort);
        

        nvgEndFrame(nvg);

        uiFrameEnd();
        networkUpdate();
        networkFlushWrites();
        SDL_WaitEventTimeout(NULL, 16);
	SDL_GL_SwapWindow(wnd);
    }

    coreShutdown();
    networkClose();
    nvgDeleteGL3(nvg);

    SDL_DestroyWindow(sdl_wnd);
    SDL_Quit();
    return 0;
}
