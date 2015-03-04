#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define ASSERT(n) assert(n)

#include "types.h"
#include "gfx.h"
#include "util.h"
#include "primitives.h"
#include "linmath.h"

#include "gl44.h"

#include "stb_image.h"

#define MAX_VBOS 16
#define MAX_IBOS 16
#define MAX_UBOS 16
#define MAX_TEXTURES 16
#define MAX_SHADERS 16
#define MAX_PIPELINES 16
#define MAX_FBOS 16
typedef struct GfxContext {
  uint32 vbo[MAX_VBOS];
  uint32 ibo[MAX_IBOS];
  uint32 ubo[MAX_UBOS];
  uint32 tex[MAX_TEXTURES];
  uint32 programs[MAX_SHADERS];
  uint32 pipelines[MAX_PIPELINES];
  uint32 fbos[MAX_FBOS];
  uint32 vtxformats[VERTEX_FORMATS];
  uint16 vboCnt;
  uint16 iboCnt;
  uint16 uboCnt;
  uint16 texCnt;
  uint16 shaderCnt;
  uint16 pipelineCnt;
  uint16 fboCnt;
} GfxContext;


static GfxContext gctx;

static void initVertexFormats()
{
  // Specify vertex formats using ARB_vertex_attrib_binding
  glBindVertexArray(gctx.vtxformats[VERT_POS_NOR]);  
  // Position
  glVertexAttribFormat(0,                 // Attribute index layout (location = 0) for pos
                       3,                 // Size 3 * floats
                       GL_FLOAT,          // Type of attrib
                       GL_FALSE,          // Normalised?
                       0                  // Offset
                       );
  glVertexAttribBinding(0, 0);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);            // NORMAL
  glVertexAttribBinding(1, 1);

  glBindVertexArray(gctx.vtxformats[VERT_POS_NOR_T0]);
  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);            // POS
  glVertexAttribBinding(0, 0);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);            // NORMAL
  glVertexAttribBinding(1, 1);
  glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);            // TEXCOORD0
  glVertexAttribBinding(2, 2);

  glBindVertexArray(gctx.vtxformats[VERT_POS_NOR_STRIDED]);
  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);                            // POS
  glVertexAttribBinding(0, 0);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));            // NORMAL
  glVertexAttribBinding(1, 0);

  glBindVertexArray(gctx.vtxformats[VERT_POS_NOR_T0_STRIDED]);
  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);                            // POS
  glVertexAttribBinding(0, 0);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));            // NORMAL
  glVertexAttribBinding(1, 0);
  glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));            // TEXCOORD0
  glVertexAttribBinding(2, 0);

  glBindVertexArray(gctx.vtxformats[VERT_POS_T0_STRIDED]);
  glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0 * sizeof(float));             // POS
  glVertexAttribBinding(0, 0);
  glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));            // NORMAL
  glVertexAttribBinding(1, 0);

  glBindVertexArray(0);
}

struct BlitTextureQuad {
  RenderPacket rp;
  int8 texLocation; // Uniform location in shader
};
static void initBlitTextureQuad(struct BlitTextureQuad* quad, const char* vsh, const char* fsh)
{
  ssquadInit(&quad->rp, vsh, fsh);
  quad->texLocation = glGetUniformLocation(quad->rp.fsh, "tex");
}

static struct BlitTextureQuad s_blitQuadTEX;
static struct BlitTextureQuad s_blitQuadFBO;
void gfxInit()
{
  memset(&gctx, 0, sizeof(GfxContext));
  // Generate resource names
  glGenBuffers(MAX_VBOS, (uint32*)&gctx.vbo);
  glGenBuffers(MAX_IBOS, (uint32*)&gctx.ibo);
  glGenBuffers(MAX_UBOS, (uint32*)&gctx.ubo);
  glGenTextures(MAX_TEXTURES, (uint32*)&gctx.tex);
  glGenProgramPipelines(MAX_PIPELINES, (uint32*)&gctx.pipelines);
  glGenFramebuffers(MAX_FBOS, (uint32*)&gctx.fbos);
  glGenVertexArrays(VERTEX_FORMATS, (uint32*)&gctx.vtxformats);

  // Enable debug output
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(debugOutputCallback, 0);

  initVertexFormats();
  initBlitTextureQuad(&s_blitQuadTEX, "shaders/quad_tex.vert", "shaders/ssquad.frag");
  initBlitTextureQuad(&s_blitQuadFBO, "shaders/quad_fbo.vert", "shaders/ssquad.frag");
}

uint32 gfxCreateVBO(void* data, uint32 size)
{
    uint32 vbo = gctx.vbo[gctx.vboCnt++];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return vbo;
}

uint32 gfxCreateIBO(void* data, uint32 size)
{
    uint32 ibo = gctx.ibo[gctx.iboCnt++];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return ibo;
}

uint32 gfxCreateUBO(uint32 size)
{
  uint32 ubo = gctx.ubo[gctx.uboCnt++];
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, 0, GL_DYNAMIC_DRAW);
  return ubo;
}

void gfxUniformBindingPoint(uint32 shader, const char* uniformBlockName, uint32 bindingPoint)
{
    uint32 uboIndex = glGetUniformBlockIndex(shader, uniformBlockName); 
    glUniformBlockBinding(shader, uboIndex, bindingPoint);
}

void gfxBindUniformBuffer(uint32 ubo, void* data, size_t size, uint32 bindingPoint)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}
void gfxBindUniformBuffers(uint32* ubos, void** data, size_t* sizes, uint8 numBuffers)
{
  for (uint8 i=0; i < numBuffers; i++) {
    glBindBufferBase(GL_UNIFORM_BUFFER, i, ubos[i]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizes[i], data[i]);
  }
}

void gfxVertexFormat(uint8 vertexFormat) {
  glBindVertexArray(gctx.vtxformats[vertexFormat]);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  if (vertexFormat > VERT_POS_T0_STRIDED) {
    glEnableVertexAttribArray(2);
  }
}

void gfxBindVertexBuffer(uint32 vbo, uint8 bindingPoint, uint8 stride) {
  glBindVertexBuffer(bindingPoint, vbo, 0, stride);
}

void gfxBindIndexBuffer(uint32 ibo) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

typedef struct TextureFormatsGL {
  uint32 sizedInternalFormat;
  uint32 baseInternalFormat;
} TextureFormatsGL;

static const uint8 s_requestedComponents[TEXTURE_FORMATS] = 
{
   1,  //TEX_R8
   1,  //TEX_R16
   1,  //TEX_R16F
   1,  //TEX_R32
   1,  //TEX_R32F
   2,  //TEX_RG8
   2,  //TEX_RG16
   2,  //TEX_RG16F
   2,  //TEX_RG32
   2,  //TEX_RG32F
   4,  //TEX_RGBA8
   4,  //TEX_RGBA16
   4,  //TEX_RGBA16F
   4,  //TEX_RGBA32
   4,  //TEX_RGBA32F
   0,  //TEX_D16F
   0,  //TEX_D24F
   0,  //TEX_D32F
   0,  //TEX_D24S8
};

static const TextureFormatsGL s_textureFormats[TEXTURE_FORMATS] =
{
  { GL_R8	,	GL_RED },
  { GL_R16	,	GL_RED },
  { GL_R16F	,	GL_RED },
  { GL_R32UI	,	GL_RED },
  { GL_R32F	,	GL_RED },
  { GL_RG8	,	GL_RG  },
  { GL_RG16	,	GL_RG },
  { GL_RG16F	,	GL_RG },
  { GL_RG32UI	,	GL_RG },
  { GL_RG32F	,	GL_RG },
  { GL_RGBA8	,	GL_RGBA },
  { GL_RGBA16	,	GL_RGBA },
  { GL_RGBA16F	,	GL_RGBA },
  { GL_RGBA32UI	,	GL_RGBA },
  { GL_RGBA32F	,	GL_RGBA },
  
  { GL_DEPTH_COMPONENT16	,GL_DEPTH_COMPONENT},
  { GL_DEPTH_COMPONENT24	,GL_DEPTH_COMPONENT},
  { GL_DEPTH_COMPONENT32	,GL_DEPTH_COMPONENT},
  { GL_DEPTH24_STENCIL8		,GL_DEPTH_STENCIL},
};

uint32 gfxCreateImage2D(uint16 w, uint16 h, uint8 texFormat) {
    TextureFormatsGL format = s_textureFormats[texFormat];
    glBindTexture(GL_TEXTURE_2D, gctx.tex[gctx.texCnt]);
    glTexImage2D( GL_TEXTURE_2D, 
                0,                                                      // level
                s_textureFormats[texFormat].sizedInternalFormat,        // internalFormat
                w,                                                  // width
                h,                                                 // height
                0,                                                      // border must be 0
                s_textureFormats[texFormat].baseInternalFormat,         // format
                GL_UNSIGNED_BYTE,                                       // type of pixel data
                0);                                                     // data
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return gctx.tex[gctx.texCnt++];
}

uint32 gfxCreateTexture2D(const char* filename, uint16* w, uint16* h, uint8 texFormat, uint8 numMips)
{
    ASSERT(texFormat < TEX_D16F);
    int x,y,n;
    uint8 levels = numMips + 1;
    TextureFormatsGL format = s_textureFormats[texFormat];
    uint8 numComponents = s_requestedComponents[texFormat];

    unsigned char *data = stbi_load(filename, &x, &y, &n, numComponents);
    ASSERT(data != NULL);
    if (w && h) {
      *w = x; *h = y;
    }

    glBindTexture(GL_TEXTURE_2D, gctx.tex[gctx.texCnt]);
    // Allocates storage for #levels mips
    glTexStorage2D(GL_TEXTURE_2D, levels, format.sizedInternalFormat, x, y);

    if (texFormat < TEX_D16F) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, format.baseInternalFormat, GL_UNSIGNED_BYTE, data);
    }
    
    if (numMips > 0) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return gctx.tex[gctx.texCnt++];
}

static const uint32 s_shaderTypes[SHADER_COUNT] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER, GL_GEOMETRY_SHADER };

static char s_shaderLog[4096];
uint32 gfxCreateShaderSource(const char* src, uint8 shaderType) {
  uint32 program = glCreateShaderProgramv(s_shaderTypes[shaderType], 1, &src);
  int32 length;
  glGetProgramInfoLog(program, 4096, &length, s_shaderLog);
  if (length) {
    printf("Shader compilation failed:\n%s\n", s_shaderLog);
    return 0;
  }
  gctx.programs[gctx.shaderCnt++] = program;
  return program;
}

uint32 gfxCreateShader(const char* filename, uint8 shaderType) {
  char* shaderSrc;
  size_t srcSize;
  int32 length;
  int ret = load_file(filename, &shaderSrc, &srcSize);
  ASSERT(ret == FILELOAD_SUCCESS);

  uint32 program = glCreateShaderProgramv(s_shaderTypes[shaderType], 1, (const char**)&shaderSrc);
  glGetProgramInfoLog(program, 4096, &length, s_shaderLog);
  if (length) {
    printf("Shader compilation failed for %s:\n%s\n", filename, s_shaderLog);
    return 0;
  }
  gctx.programs[gctx.shaderCnt++] = program;
          
  return program;
}

uint32 gfxCreatePipeline() {
  return gctx.pipelines[gctx.pipelineCnt++];
}
void gfxBindPipeline(uint32 pipeline) {
  glBindProgramPipeline(pipeline);
}

void gfxReplaceShaders(uint32 pipeline, uint32 vert, uint32 frag)
{
  glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vert);
  glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, frag);
}
void gfxReplaceShadersGeom(uint32 pipeline, uint32 vert, uint32 frag, uint32 geom)
{
  glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vert);
  glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, frag);
  glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, geom);
}
void gfxReplaceVertexShader(uint32 pipeline, uint32 vert) {
  glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vert);
}
void gfxReplaceFragShader(uint32 pipeline, uint32 frag) {
  glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, frag);
}
void gfxReplaceGeomShader(uint32 pipeline, uint32 geom) {
  glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, geom);
}
void gfxReplaceComputeShader(uint32 pipeline, uint32 comp) {
  glUseProgramStages(pipeline, GL_COMPUTE_SHADER_BIT, comp);
}

uint32 gfxCreateFramebuffer(uint16 width, uint16 height, uint8 colorFormat, uint8 depthFormat, uint32* colorTex, uint32* depthTex)
{
  uint32 fbo = gctx.fbos[gctx.fboCnt++];
  uint32 color = gctx.tex[gctx.texCnt++];
  uint32 depth = gctx.tex[gctx.texCnt++];
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  // Create color texture(buffer)
  glBindTexture(GL_TEXTURE_2D, color);
  glTexImage2D( GL_TEXTURE_2D, 
                0,                                                      // level
                s_textureFormats[colorFormat].sizedInternalFormat,      // internalFormat
                width,                                                  // width
                height,                                                 // height
                0,                                                      // border must be 0
                s_textureFormats[colorFormat].baseInternalFormat,       // format
                GL_UNSIGNED_BYTE,                                       // type of pixel data
                0);                                                     // data
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

  // Create depth buffer
  glBindTexture(GL_TEXTURE_2D, depth);
  glTexImage2D( GL_TEXTURE_2D, 
                0,                                                      // level
                s_textureFormats[depthFormat].sizedInternalFormat,      // internalFormat
                width,                                                  // width
                height,                                                 // height
                0,                                                      // border must be 0
                s_textureFormats[depthFormat].baseInternalFormat,       // format
                GL_UNSIGNED_BYTE,                                       // type of pixel data
                0);                                                     // data
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, 
      depthFormat >= TEX_D24S8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, 
      GL_TEXTURE_2D, depth, 0);

  if (colorTex)
    *colorTex = color;
  
  if (depthTex)
    *depthTex = depth;

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return fbo;
}

void gfxBindFramebuffer(uint32 fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void gfxResizeTexture(uint32 tex, uint8 format, uint32 width, uint32 height)
{
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D( GL_TEXTURE_2D, 
                0,                                                      // level
                s_textureFormats[format].sizedInternalFormat,           // internalFormat
                width,                                                  // width
                height,                                                 // height
                0,                                                      // border must be 0
                s_textureFormats[format].baseInternalFormat,            // format
                GL_UNSIGNED_BYTE,                                       // type of pixel data
                0);                                                     // data
}

void gfxShutdown() {
  glDeleteBuffers(MAX_VBOS, (uint32*)&gctx.vbo);
  glDeleteBuffers(MAX_IBOS, (uint32*)&gctx.ibo);
  glDeleteVertexArrays(VERTEX_FORMATS, (uint32*)&gctx.vtxformats);
  glDeleteTextures(MAX_TEXTURES, (uint32*)&gctx.tex);
  glDeleteProgramPipelines(MAX_PIPELINES, (uint32*)&gctx.pipelines);
  glDeleteFramebuffers(MAX_FBOS, (uint32*)&gctx.fbos);

  for (uint16 ii=0; ii<gctx.shaderCnt; ii++) {
    glDeleteProgram(gctx.programs[ii]);
  }
}

void gfxBindImage2D(uint32 image, uint32 img_unit, uint8 format) {
  // Shaders themselves specify usage
  glBindImageTexture(img_unit, image, 0, GL_FALSE, 0, GL_READ_WRITE, s_textureFormats[format].sizedInternalFormat);
}

void gfxBindTextures2D(uint32* texs, int8* locations, uint8 numTextures, uint32 program) {
  for (uint8 i=0; i<numTextures; i++)
  {
    if (locations[i] == -1) // Invalid location - uniform not active
      continue;
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, texs[i]);
    glProgramUniform1i(program, 0, locations[i]);
  }
}

void gfxBlitFramebuffer(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h)
{
    mat4x4 ortho;
    mat4x4_ortho(ortho, 0.f, wnd_w, wnd_h, 0.f, 0.f, 100.f);
    gfxBindTextures2D(&tex, &s_blitQuadFBO.texLocation, 1, s_blitQuadFBO.rp.fsh);
    ssquadResize(&s_blitQuadFBO.rp, ortho, x, y, w, h);
    ssquadDraw(&s_blitQuadFBO.rp);
}

void gfxBlitTexture(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h)
{
    mat4x4 ortho;
    mat4x4_ortho(ortho, 0.f, wnd_w, wnd_h, 0.f, 0.f, 100.f);
    gfxBindTextures2D(&tex, &s_blitQuadTEX.texLocation, 1, s_blitQuadTEX.rp.fsh);
    ssquadResize(&s_blitQuadTEX.rp, ortho, x, y, w, h);
    ssquadDraw(&s_blitQuadTEX.rp);
}

int32 s_uniformTypeMap[UNIFORM_TYPES] = {
  GL_FLOAT,
  GL_FLOAT_VEC2,
  GL_FLOAT_VEC3,
  GL_FLOAT_VEC4,
  GL_FLOAT_MAT2,
  GL_FLOAT_MAT3,
  GL_FLOAT_MAT4,
  GL_SAMPLER_1D,
  GL_SAMPLER_2D,
  GL_SAMPLER_3D,
  GL_SAMPLER_CUBE,
  GL_IMAGE_1D,
  GL_IMAGE_2D,
  GL_IMAGE_2D_ARRAY,
  GL_IMAGE_3D,
};

static uint8 s_uniformName[128];
int32 gfxGetShaderUniforms(uint32 program, uint8* namesBuffer, uint32 namesBufferSize, uint8* types, uint8* locations) {
  uint32 namesBufferOffset = 0;
  GLint numUniforms = 0;
  glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
  const GLenum properties[4] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};
   
  for(int ii = 0; ii < numUniforms; ++ii)
  {
    GLint values[4];
    glGetProgramResourceiv(program, GL_UNIFORM, ii, 4, properties, 4, NULL, values);

    //Skip any iiorms that are in a block.
    // values[0] = GL_BLOCK_INDEX
    if(values[0] != -1)
      continue;


    // values[2] = GL_NAME_LENGTH
    int32 len = values[2] + 1;
    if (namesBufferOffset + len > namesBufferSize) {
      break;
    }
    glGetProgramResourceName(program, GL_UNIFORM, ii, 128, NULL, s_uniformName);
    
    // Write to out buffers
    memcpy(namesBuffer+namesBufferOffset, s_uniformName, len);
    namesBufferOffset += len;
    locations[ii] = (uint8)values[3];

    uint8 type = UNIFORM_TYPES;
    for (uint8 t=0; t < UNIFORM_TYPES; t++) {
      if (values[1] == s_uniformTypeMap[t]) {
        type = t;
        break;
      }
    }
    types[ii] = type;
  }
  return numUniforms;
}

void gfxSetImageUnit(uint32 program, uint32 imageLocation, int32 imgUnit)
{
  glProgramUniform1i(program, imageLocation, imgUnit);
}

void gfxDispatchCompute(uint32 x, uint32 y, uint32 z)
{
    glDispatchCompute(x,y,z);
}
