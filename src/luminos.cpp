/*
 * Copyright 2014 Onat Turkcuoglu. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include <bx/timer.h>

#include <SDL2/SDL.h>
#undef main

#include <bgfx.h>
#include <bgfxplatform.h>

#include "lua_xforms.h"

#include "nanovg/nanovg.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"
#include "ui_xforms.h"

static char status_msg[256] = {0};
static char error_msg[2048] = {0};
static char stdout_msg[2048] = {0};

RecompileInput rc_in =
{
    "scripts/program.lua",
    status_msg,
    error_msg
};

bool quit = false;

int main(int _argc, char** _argv)
{
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t debug = BGFX_DEBUG_TEXT;
    uint32_t reset = BGFX_RESET_VSYNC;

	SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* wnd = SDL_CreateWindow("luminos"
                    , SDL_WINDOWPOS_UNDEFINED
                    , SDL_WINDOWPOS_UNDEFINED
                    , width
                    , height
                    , SDL_WINDOW_SHOWN
                    | SDL_WINDOW_RESIZABLE
                    );

    bgfx::sdlSetWindow(wnd);

    bgfx::init();
    bgfx::reset(width, height);

    NVGcontext* nvg = nvgCreate(512, 512, 1, 0);
	bgfx::setViewSeq(0, true);

	bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
	bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png"));

    // Enable debug text.
    bgfx::setDebug(debug);

    // Set view 0 clear state.
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
        , 0x303030ff
        , 1.0f
        , 0
        );
	int64_t timeOffset = bx::getHPCounter();

    // Setup Lua
    initLua();
    ui_init();
    cmdRecompile((const void*)&rc_in);
    ui_setNVGContext(nvg);
	initEnvironmentVariables();

	SDL_Event event;
    //while (!entry::processEvents(width, height, debug, reset, &mouse_state) )
    while (!quit)
    {
        while (SDL_PollEvent(&event) )
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        const Uint8 *state = SDL_GetKeyboardState(NULL);

		int64_t now = bx::getHPCounter();
		const double freq = double(bx::getHPFrequency() );
		float time = (float)( (now-timeOffset)/freq);

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, width, height);

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::submit(0);

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, status_msg);
        bgfx::dbgTextPrintf(0, 2, 0x6f, error_msg);
        bgfx::dbgTextPrintf(0, 3, 0x6f, stdout_msg);
        if (state[SDL_SCANCODE_RETURN]) {
            bgfx::dbgTextPrintf(0, 4, 0x4f, "Return is pressed");
        }
        if (state[SDL_SCANCODE_RIGHT] && state[SDL_SCANCODE_UP]) {
            bgfx::dbgTextPrintf(0, 4, 0x4f, "Ayayayayaya");
        }

		uploadEnvironmentVariables(time);
        port_programStart("portProgramStart", stdout_msg);

        nvgBeginFrame(nvg, width, height, 1.0f, NVG_STRAIGHT_ALPHA);

        for (int i = 0; i < 3; i++)
        {
			float k  =(float)i;
            bndNodePort(nvg, 640 + k * 50, 100 + k * 90, (BNDwidgetState)i, nvgRGBA(255, 50, 100, 255));
            bndNodeWire(nvg, 640 + k * 50, 100 + k * 90, 800 + k * 50, 400 + k * 90, (BNDwidgetState)i, BNDwidgetState::BND_DEFAULT);
            //bndNodeIconLabel(nvg, 300 + i * 300, 300, 200 * scale, 100 * scale, BND_ICONID(2, 11), nvgRGBA(255, 50, 100, 255), nvgRGBA(255, 50, 100, 10), 1, 20, "Check out");
            bndNodeBackground(nvg, 300 + k * 300, 400, 200, 100, (BNDwidgetState)i, BND_ICONID(5, 11), "Node Background", nvgRGBA(255, 50, 100, 255));
            //bndNodeIconLabel(nvg, 300 + i * 300, 490, 200, 100, BND_ICONID(2, 11), nvgRGBA(255, 50, 100, 255), nvgRGBA(255, 50, 100, 10), 1, 40, "Check out");
        }

		nvgEndFrame(nvg);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();
    }

    // Shutdown bgfx.
    bgfx::shutdown();
	shutdownLua();

    SDL_DestroyWindow(wnd);
    SDL_Quit();
    return 0;
}
