#include <stdlib.h>

#include "types.h"
#include "gfx.h"
#include "gl44.h"

#include "primitives.h"

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


void cubeInit(CubeRenderPacket* cube, const char* vsh, const char* fsh)
{
    cube->rp.vbo     = gfxCreateVBO(s_cubeVertices, sizeof(s_cubeVertices));
    cube->rp.ibo     = gfxCreateIBO(s_cubeIndices , sizeof(s_cubeIndices));
    cube->ubo        = gfxCreateUBO(sizeof(struct Transforms));
    cube->rp.vsh     = gfxCreateShader(vsh, SHADER_VERT);
    cube->rp.fsh     = gfxCreateShader(fsh, SHADER_FRAG);
    cube->rp.pipe    = gfxCreatePipeline();

    gfxReplaceShaders(cube->rp.pipe, cube->rp.vsh, cube->rp.fsh);
    gfxUniformBindingPoint(cube->rp.vsh, "Transforms", 0);
}

void cubeDraw(CubeRenderPacket* cube)
{
    gfxVertexFormat(VERT_POS_NOR_STRIDED);
    gfxBindVertexBuffer(cube->rp.vbo, 0, sizeof(struct PosNormalVertex));
    gfxBindIndexBuffer(cube->rp.ibo);
    gfxBindPipeline(cube->rp.pipe);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void cubeUpdate(CubeRenderPacket* cube, vec3 rot, float angle, float x, float y, float z, float* view, float* proj)
{
    static mat4x4 temp;
    memcpy(cube->transforms.view, view, 16 * sizeof(float));
    memcpy(cube->transforms.proj, proj, 16 * sizeof(float));

    vec3_norm(rot, rot);

    mat4x4_identity(cube->transforms.world);
    mat4x4_translate_in_place(cube->transforms.world, x, y, z);
    mat4x4_rotate(cube->transforms.world, cube->transforms.world, rot[0], rot[1], rot[2], angle);

    mat4x4_mul(temp, cube->transforms.view, cube->transforms.world);
    mat4x4_mul(cube->transforms.proj_view_world, cube->transforms.proj, temp);
    gfxBindUniformBuffer(cube->ubo, &cube->transforms, sizeof(struct Transforms), 0);
}
