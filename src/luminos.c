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

static bool quit = false;

struct PosNormalVertex {
    float pos[3];
    float normal[3];
};

struct PosTexcoord0Vertex {
    float pos[2];
    float texcoord[2];
};

static const uint32 s_cubeIndices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 1, 5,
    0, 5, 4,
    0, 3, 4,
    3, 7, 4,
    1, 2, 5,
    5, 2, 6,
    3, 2, 6,
    3, 6, 7,
    4, 5, 6,
    4, 6, 7
};

static struct PosNormalVertex s_cubeVertices[] = {
    {{-1.f,  1.f, -1.f}, { -.57735f,  .57735f,  -.57735f}},
    {{-1.f,  1.f,  1.f}, { -.57735f,  .57735f,   .57735f}},
    {{ 1.f,  1.f,  1.f}, {  .57735f,  .57735f,   .57735f}},
    {{ 1.f,  1.f, -1.f}, {  .57735f,  .57735f,  -.57735f}},
    {{-1.f, -1.f, -1.f}, { -.57735f, -.57735f,  -.57735f}},
    {{-1.f, -1.f,  1.f}, { -.57735f, -.57735f,   .57735f}},
    {{ 1.f, -1.f,  1.f}, {  .57735f, -.57735f,   .57735f}},
    {{ 1.f, -1.f, -1.f}, {  .57735f, -.57735f,  -.57735f}},
};



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
    //uint32 vbo = gfxCreateVBO(s_cubeVertices, sizeof(s_cubeVertices));
    //uint32 ibo = gfxCreateIBO(s_cubeIndices, sizeof(s_cubeIndices));
    uint32 fsh = gfxCreateShader("shaders/ssquad.frag", SHADER_FRAG);
    uint32 blinn = gfxCreatePipeline();
    uint32 tex = gfxCreateTexture2D("textures/doge.png", 0, 0, TEX_RGBA8, 0);
    uint32 location = glGetUniformLocation(fsh, "tex");
    gfxReplaceFragShader(blinn, fsh);

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

        gfxBindSSQuad(blinn);
        gfxBindPipeline(blinn);
        gfxBindTextures2D(&tex, &location, 1, fsh);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        /*
        quit |= uiFrameStart(width, height);
        if (!s_errorPort)
            coreExecPort("portProgramStart");
        else
            coreExecPort(s_errorPort);

        uiFrameEnd();
        */
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
