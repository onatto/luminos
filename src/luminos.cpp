/*
 * Copyright 2014 Onat Turkcuoglu. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include <bx/timer.h>

#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_X11
#undef SDL_VIDEO_DRIVER_WINDOWS
#undef main

#include <bgfx.h>
#include <bgfxplatform.h>

#include "core_xforms.h"
extern "C" {
#include "network.h"
}

#include "nanovg/nanovg.h"
#include "lua.hpp"
#include "lauxlib.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"
#include "ui_xforms.h"

static bool quit = false;

int main(int /*_argc*/, char** /*_argv*/)
{
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t debug = BGFX_DEBUG_TEXT;

    SDL_Init(SDL_INIT_VIDEO);
    sdl_wnd = SDL_CreateWindow("luminos"
                    , 0
                    , 0
                    , width
                    , height
                    , SDL_WINDOW_SHOWN
                    | SDL_WINDOW_RESIZABLE
                    );

    bgfx::sdlSetWindow(sdl_wnd);

    bgfx::init();
    bgfx::reset(width, height);

    NVGcontext* nvg = nvgCreate(2048, 2048, 1, 0);
    ui_setNVGContext(nvg);

	bgfx::setViewSeq(0, true);

	bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
	bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png"));

    // Enable debug text.
    bgfx::setDebug(debug);

    // Set view 0 clear state.
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
        , 0x303030ff
        , 1.0f
        , 0
        );
	int64_t timeOffset = bx::getHPCounter();

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
                    bgfx::reset(width, height);
                    break;
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
               ui_textInputEvent(&event); 
            }

        }
        if (!quit)
            quit = ui_frameStart();

		int64_t now = bx::getHPCounter();
		const double freq = double(bx::getHPFrequency() );
		float time = (float)( (now-timeOffset)/freq);

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, width, height);

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::submit(0);

        core_updateGlobals(time);
        if (!s_errorPort)
            core_execPort("portProgramStart");
        else
            core_execPort(s_errorPort);

        nvgBeginFrame(nvg, width, height, 1.0f, NVG_STRAIGHT_ALPHA);
		nvgEndFrame(nvg);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();
        ui_frameEnd();
        network_update();
        // Flush writes at the end of the frame
        network_flushw();
        SDL_WaitEventTimeout(NULL, 16);
    }

    // Shutdown bgfx.
    bgfx::shutdown();
    core_shutdown();
    network_close();

    SDL_DestroyWindow(sdl_wnd);
    SDL_Quit();
    return 0;
}
