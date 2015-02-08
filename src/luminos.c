/*
 * Copyright 2014 Onat Turkcuoglu. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_X11
#undef SDL_VIDEO_DRIVER_WINDOWS
#undef main

#include "types.h"
#include "core_xforms.h"
#include "network.h"

#include "nanovg/nanovg.h"

#include "gl_core_4_4.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

#include "lua.h"
#include "lauxlib.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"
#include "ui_xforms.h"


static bool quit = false;
void wnd_resizeWindow(uint32 width, uint32 height)
{
    glViewport(0,0,width,height);
}

SDL_Window* wnd_initSDL(uint32 width, uint32 height) 
{
    SDL_Window* sdl_wnd;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    sdl_wnd = SDL_CreateWindow("luminos"
                    , 0
                    , 0
                    , width
                    , height
                    , SDL_WINDOW_SHOWN
                    | SDL_WINDOW_RESIZABLE
                    | SDL_WINDOW_OPENGL
                    );

    if (sdl_wnd == NULL) {
	printf("Couldn't create SDL window. %s\n", SDL_GetError());
	return NULL;
    }

    return sdl_wnd;
}

SDL_GLContext wnd_initGL(SDL_Window* wnd)
{
    SDL_GLContext glcontext = SDL_GL_CreateContext(wnd);
    if(ogl_LoadFunctions() == ogl_LOAD_FAILED) {
        printf("Loading GL functions failed.\n");
        return NULL;
    }
    return glcontext;
}

int main(int _argc, char** _argv)
{
    uint32_t width = 1920;
    uint32_t height = 1080;

    SDL_Window* wnd = wnd_initSDL(width, height);
    SDL_GLContext glcontext = wnd_initGL(wnd);

    NVGcontext* nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    ui_setNVGContext(nvg);

    bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
    bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png", 0));

    glClearColor(0.1,0.1,0.1,1);
    glClearStencil(0);
    glEnable(GL_STENCIL_FUNC);
/*    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
        , 0x303030ff
        , 1.0f
        , 0
        );*/
	int64_t timeOffset = SDL_GetPerformanceCounter();

    // Setup Lua
    core_init();

    cmd_restart("scripts/program.lua");
    ui_init();
    core_execPort("portProgramInit");

    lua_getglobal(get_luaState(), "serverIP");
    const char* server_ip = lua_tolstring(get_luaState(), -1, NULL);

    network_init(get_luaState(), server_ip, 3333);

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
                    wnd_resizeWindow(width, height);
                    break;
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
               ui_textInputEvent(&event); 
            }

        }
        if (!quit) {
            quit = ui_frameStart();
        }

        int64 now = SDL_GetPerformanceCounter();
        const double freq = SDL_GetPerformanceFrequency();
        float time = (float)( (now-timeOffset)/freq);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        // Set view 0 default viewport.
        //bgfx::setViewRect(0, 0, 0, width, height);

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        //bgfx::submit(0);

        core_updateGlobals(time);

        nvgBeginFrame(nvg, width, height, 1.2f);
        
        if (!s_errorPort)
            core_execPort("portProgramStart");
        else
            core_execPort(s_errorPort);
        

        nvgEndFrame(nvg);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        //bgfx::frame();
        ui_frameEnd();
        network_update();
        // Flush writes at the end of the frame
        network_flushw();
        SDL_WaitEventTimeout(NULL, 16);
	SDL_GL_SwapWindow(wnd);
    }

    // Shutdown bgfx.
    //bgfx::shutdown();
    core_shutdown();
    network_close();
    nvgDeleteGL3(nvg);

    SDL_DestroyWindow(sdl_wnd);
    SDL_Quit();
    return 0;
}
