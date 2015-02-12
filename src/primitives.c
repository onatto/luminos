#include <stdlib.h>
#include <string.h>

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


void cubeInit(CubeRenderPacket* cube, const char* vsh_path, const char* fsh_path)
{
    cube->rp.vbo     = gfxCreateVBO((void*)s_cubeVertices, sizeof(s_cubeVertices));
    cube->rp.ibo     = gfxCreateIBO((void*)s_cubeIndices , sizeof(s_cubeIndices));
    cube->ubo        = gfxCreateUBO(sizeof(struct Transforms));
    cube->rp.vsh     = gfxCreateShader(vsh_path, SHADER_VERT);
    cube->rp.fsh     = gfxCreateShader(fsh_path, SHADER_FRAG);
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

static const float ssquad_vertices[] = {
  -1.f,   1.f,  0.f, 0.f ,
   1.f,   1.f,  1.f, 0.f ,
  -1.f,  -1.f,  0.f, 1.f ,
   1.f,  -1.f,  1.f, 1.f ,
};
static const uint32 ssquad_indices[] = {
  1, 0, 2,
  1, 2, 3
};

void ssquadInit(RenderPacket* ssquad, const char* vsh_path, const char* fsh_path)
{
    ssquad->vbo        = gfxCreateVBO((void*)ssquad_vertices, sizeof(ssquad_vertices));
    ssquad->ibo        = gfxCreateIBO((void*)ssquad_indices, sizeof(ssquad_indices));
    ssquad->vsh     = gfxCreateShader(vsh_path, SHADER_VERT);
    ssquad->fsh     = gfxCreateShader(fsh_path, SHADER_FRAG);
    ssquad->pipe    = gfxCreatePipeline();

    gfxReplaceShaders(ssquad->pipe, ssquad->vsh, ssquad->fsh);
}

void ssquadDraw(RenderPacket* ssquad) {
  gfxVertexFormat(VERT_POS_T0_STRIDED);
  gfxBindVertexBuffer(ssquad->vbo, 0, 4 * sizeof(float));
  gfxBindIndexBuffer(ssquad->ibo);
  gfxBindPipeline(ssquad->pipe);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void ssquadResize(RenderPacket* ssquad, float* proj, float x, float y, float w, float h)
{
    uint32 proj_loc   = glGetUniformLocation(ssquad->vsh, "proj");
    uint32 offset_loc = glGetUniformLocation(ssquad->vsh, "offset");
    glProgramUniformMatrix4fv(ssquad->vsh, proj_loc, 1, false, proj);
    glProgramUniform4f(ssquad->vsh, offset_loc, x, y, w, h);
}
