#pragma once

#include "linmath.h"

struct PosNormalVertex {
    float pos[3];
    float normal[3];
};

struct Transforms {
    mat4x4 world;
    mat4x4 view;
    mat4x4 proj;
    mat4x4 proj_view_world;
    vec4   camera_wspos;
};

typedef struct RenderPacket {
    uint32_t vbo;
    uint32_t ibo;
    uint32_t vsh;
    uint32_t fsh;
    uint32_t pipe;
} RenderPacket;

typedef struct CubeRenderPacket {
    struct RenderPacket rp;
    uint32_t ubo;
    struct Transforms transforms;
} CubeRenderPacket;

void cubeInit(CubeRenderPacket* cube, const char* vsh_path, const char* fsh_path);
void cubeDraw(CubeRenderPacket* cube);
void cubeUpdate(CubeRenderPacket* cube, vec3 rot, float angle, float x, float y, float z, float* view, float* proj);
void ssquadInit(RenderPacket* ssquad, const char* vsh_path, const char* fsh_path);
void ssquadDraw(RenderPacket* ssquad);
void ssquadResize(RenderPacket* ssquad, float* proj, float x, float y, float w, float h);
