/*
 * Copyright 2014 Onat Turkcuoglu. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include <SDL2/SDL.h>
#define SDL_VIDEO_DRIVER_X11
#undef SDL_VIDEO_DRIVER_WINDOWS
#undef main


#include "gl44.h"
#include "lua.h"
#include "lauxlib.h"

#include "types.h"
#include "core.h"
#include "network.h"
#include "windowing.h"
#include "gfx.h"
#include "ui.h"
#include "linmath.h"
#include "primitives/primitives.h"

static bool quit = false;

struct PosTexcoord0Vertex {
    float pos[2];
    float texcoord[2];
};

int main(int _argc, char** _argv)
{
    UNUSED(_argc);
    UNUSED(_argv);
    uint32_t width = 1920;
    uint32_t height = 1080;

    SDL_Window* wnd = wndInitSDL(width, height);
    wndInitGL(wnd);

    glClearColor(0.3,0.3,0.3,1);
    glClearStencil(0);
    glEnable(GL_STENCIL_FUNC);
    
    // Init gfx
    gfxInit();
    CubeRenderPacket cube;
    mat4x4 view, proj;
    vec3 eye = {0.f, 0.f, -8.f};
    vec3 center = {0.f, 0.f, 0.f};
    vec3 up = {0.f, 1.f, 0.f};
    mat4x4_look_at(view, eye, center, up);
    cubeInit(&cube, "shaders/blinn.vert", "shaders/blinn.frag");

    //uint32 tex = gfxCreateTexture2D("textures/doge.png", 0, 0, TEX_RGBA8, 0);
    //uint32 location = glGetUniformLocation(fsh, "tex");

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

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        float aspect = (float)width/(float)height;
        mat4x4_perspective(proj, 1.57f * (9.f/16.f) / aspect, aspect, 0.1f, 400.f);

        vec3 rot = {sin(time), cos(time), 0.f};
        cubeUpdate(&cube, rot, time, 0.f, 0.f, -4.f, (float*)view, (float*)proj);
        cubeDraw(&cube);
        
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
