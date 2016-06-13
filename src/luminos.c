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
#include "primitives.h"

static bool quit = false;

int restartGraphics(RenderPacket* ssquad, CubeRenderPacket* cube, uint32* tex, uint32* fbo, uint32* loc, uint32* ctex, uint32* dtex)
{
    gfxInit();
    cubeInit(cube, "shaders/blinn.vert", "shaders/blinn.frag");
    ssquadInit(ssquad, "shaders/quad.vert", "shaders/ssquad.frag");
    uint16 fbo_width = 1200;
    uint16 fbo_height = 1200;
    *fbo = gfxCreateFramebuffer(fbo_width, fbo_height, TEX_RGBA8, TEX_D24F, ctex, dtex);

    *tex = gfxCreateTexture2D("textures/doge.png", 0, 0, TEX_RGBA8, 0);
    *loc = glGetUniformLocation(ssquad->fsh, "tex");
}

int main(int _argc, char** _argv)
{
    UNUSED(_argc);
    UNUSED(_argv);
    uint32_t width = 1920;
    uint32_t height = 1080;

    SDL_Window* wnd = wndInitSDL(width, height);
    wndInitGL(wnd);

    glClearColor(0,0,0,1);
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

    RenderPacket ssquad;
    ssquadInit(&ssquad, "shaders/quad.vert", "shaders/ssquad.frag");

    mat4x4 ortho;
    uint32 color_tex, depth_tex;
    uint16 fbo_width = 1200;
    uint16 fbo_height = 1200;
    uint32 fbo = gfxCreateFramebuffer(fbo_width, fbo_height, TEX_RGBA8, TEX_D24F, &color_tex, &depth_tex);

    uint32 tex = gfxCreateTexture2D("textures/doge.png", 0, 0, TEX_RGBA8, 0);
    uint32 location = glGetUniformLocation(ssquad.fsh, "tex");

    // Init core module
    coreInit("scripts/program.lua", getErrorMsg());

    // Init UI module - depends on core
    uiInit();

    // Init network module - depends on core
    lua_getglobal(getLuaState(), "serverIP");
    const char* server_ip = lua_tolstring(getLuaState(), -1, NULL);
    networkInit(getLuaState(), server_ip, 3333);
    coreExecPort("portProgramInit"); // Also sends msg "Workspace"

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
                        gfxShutdown();
                        uiShutdown();
                        restartGraphics(&ssquad, &cube, &tex, &fbo, &location, &color_tex, &depth_tex);
                        uiInit();
                        uiResize(width, height);
                        break;
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
               uiTextInputEvent(&event); 
            }
        }

        int64 now = SDL_GetPerformanceCounter();
        const double freq = SDL_GetPerformanceFrequency();
        double time = ( (now-timeOffset)/freq);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        float aspect = (float)width/(float)height;
        //mat4x4_perspective(proj, 1.57f * (9.f/16.f) / aspect, aspect, 0.1f, 400.f);
        mat4x4_perspective(proj, 1.57f, 1.f, 0.1f, 400.f);

        gfxBindFramebuffer(fbo);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, fbo_width, fbo_height);
        vec3 rot = {sin(time), cos(time), sin(time)};
        cubeUpdate(&cube, rot, time, 0.f, 0.f, -4.f, (float*)view, (float*)proj);
        cubeDraw(&cube);

        uiRenderBlur(width, height);
        gfxBindFramebuffer(0);
        glViewport(0, 0, width, height);
        quit |= uiFrameStart(width, height, time);
        if (!s_errorPort)
            coreExecPort("portProgramStart");
        else
            coreExecPort(s_errorPort);

        uiFrameEnd();

        gfxBlitFramebuffer(color_tex, (float)width-300.f, 0.f, 300.f, 300.f, width, height);
        uiVisualiserFrame((float)width-300.f, 0.f, 300.f, 300.f);

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
