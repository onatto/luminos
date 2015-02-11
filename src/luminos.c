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

static bool quit = false;

struct PosNormalVertex {
    float pos[3];
    float normal[3];
};

struct PosTexcoord0Vertex {
    float pos[2];
    float texcoord[2];
};

struct Transforms {
    mat4x4 world;
    mat4x4 view;
    mat4x4 proj;
    mat4x4 proj_view_world;
    vec4   camera_wspos;
};

static const uint32 s_cubeIndices[] = {
    0, 2, 1,
    0, 3, 2,
    0, 5, 1,
    0, 4, 5,
    0, 4, 3,
    3, 4, 7,
    1, 5, 2,
    5, 6, 2,
    3, 6, 2,
    3, 7, 6,
    4, 6, 5,
    4, 7, 6,
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

    glClearColor(0.3,0.3,0.3,1);
    glClearStencil(0);
    glEnable(GL_STENCIL_FUNC);
    
    // Init gfx
    gfxInit();
    uint32 vbo = gfxCreateVBO(s_cubeVertices, sizeof(s_cubeVertices));
    uint32 ibo = gfxCreateIBO(s_cubeIndices, sizeof(s_cubeIndices));
    uint32 vsh = gfxCreateShader("shaders/blinn.vert", SHADER_VERT);
    uint32 fsh = gfxCreateShader("shaders/blinn.frag", SHADER_FRAG);
    uint32 blinn = gfxCreatePipeline();
    //gfxReplaceFragShader(blinn, fsh);
    //

    uint32 tex = gfxCreateTexture2D("textures/doge.png", 0, 0, TEX_RGBA8, 0);
    uint32 location = glGetUniformLocation(fsh, "tex");

    struct Transforms transforms;
    mat4x4 identity, temp;
    mat4x4_identity(identity);

    uint32 ubo;
    glGenBuffers(1, &ubo);
    uint32 uboIndex = glGetUniformBlockIndex(vsh, "Transforms"); 
    glUniformBlockBinding(vsh, uboIndex, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(struct Transforms), 0, GL_DYNAMIC_DRAW);

    gfxReplaceShaders(blinn, vsh, fsh);

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

        mat4x4_rotate_Y(transforms.world, identity, time);
        mat4x4_identity(transforms.view);
        mat4x4_translate(transforms.view, 0.f, 0.f, -8.f);
        //float aspect = height<width ? (float)((float)width/(float)height) : (float)((float)height/(float)width);
        float aspect = (float)width/(float)height;
        mat4x4_perspective(transforms.proj, 1.57f * (9.f/16.f) / aspect, aspect, 0.1f, 400.f);
        mat4x4_mul(temp, transforms.view, transforms.world);
        mat4x4_mul(transforms.proj_view_world, transforms.proj, temp);

        gfxVertexFormat(VERT_POS_NOR_STRIDED);
        gfxBindVertexBuffer(vbo, 0, sizeof(struct PosNormalVertex));
        gfxBindIndexBuffer(ibo);
        //gfxBindSSQuad(blinn);
        gfxBindPipeline(blinn);
        // Update the buffer
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct Transforms) , &transforms);

        //gfxBindTextures2D(&tex, &location, 1, fsh);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
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
