/*
 * Copyright 2011-2014 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.h"
#include <bgfx.h>
#include "lua_xforms.h"

#include "nanovg/nanovg.h"

#define BLENDISH_IMPLEMENTATION
#include "blendish.h"

static char status_msg[256] = {0};
static char error_msg[2048] = {0};
static char stdout_msg[2048] = {0};

bool quit = false;

int _main_(int /*_argc*/, char** /*_argv*/)
{
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t debug = BGFX_DEBUG_TEXT;
    uint32_t reset = BGFX_RESET_VSYNC;

    bgfx::init();
    bgfx::reset(width, height);

    NVGcontext* nvg = nvgCreate(512, 512, 1, 0);
	bgfx::setViewSeq(0, true);

	bndSetFont(nvgCreateFont(nvg, "droidsans", "font/droidsans.ttf"));
	bndSetIconImage(nvgCreateImage(nvg, "images/blender_icons16.png"));

    initLua();

    strcpy(status_msg, "Compiled Lua successfully!");
    strcpy(error_msg, "Luminos - think xform");
    if (compileLua("luminos_data/program.lua", error_msg))
    {
        strcpy(status_msg, "Couldn't load file:");
    }

    port_programStart("portProgramStart", stdout_msg);

    // Enable debug text.
    bgfx::setDebug(debug);

    // Set view 0 clear state.
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
        , 0x303030ff
        , 1.0f
        , 0
        );

    while (!entry::processEvents(width, height, debug, reset) )
    {
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

        nvgBeginFrame(nvg, width, height, 1.0f, NVG_STRAIGHT_ALPHA);

        for (int i = 0; i < 3; i++)
        {
            bndNodePort(nvg, 640 + i * 50, 100 + i * 90, (BNDwidgetState)i, nvgRGBA(255, 50, 100, 255));
            bndNodeWire(nvg, 640 + i * 50, 100 + i * 90, 800 + i * 50, 400 + i * 90, (BNDwidgetState)i, BNDwidgetState::BND_ACTIVE);
            bndNodeBackground(nvg, 300 + i * 300, 600, 200, 100, (BNDwidgetState)i, BND_ICONID(5, 11), "Node Background", nvgRGBA(255, 50, 100, 255));
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
    return 0;
}
