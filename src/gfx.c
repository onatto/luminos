#include <string.h>
#include <assert.h>
#define ASSERT(n) assert(n)

#include "types.h"
#include "gfx.h"
#include "util.h"

#include "gl44.h"

#include "stb_image.h"

#define MAX_VBOS 16
#define MAX_IBOS 16
#define MAX_TEXTURES 16
#define MAX_SHADERS 16
#define MAX_PIPELINES 16
#define MAX_FBOS 16
typedef struct GfxContext {
  uint32 vbo[MAX_VBOS];
  uint32 ibo[MAX_IBOS];
  uint32 tex[MAX_TEXTURES];
  uint32 programs[MAX_SHADERS];
  uint32 pipelines[MAX_PIPELINES];
  uint32 fbos[MAX_FBOS];
  uint16 vboCnt;
  uint16 iboCnt;
  uint16 texCnt;
  uint16 shaderCnt;
  uint16 pipelineCnt;
  uint16 fboCnt;
} GfxContext;

GfxContext gctx;

void gfxInit()
{
  memset(&gctx, 0, sizeof(GfxContext));
  glGenBuffers(MAX_VBOS, (uint32*)&gctx.vbo);
  glGenBuffers(MAX_IBOS, (uint32*)&gctx.ibo);
  glGenTextures(MAX_TEXTURES, (uint32*)&gctx.tex);
  glGenProgramPipelines(MAX_PIPELINES, (uint32*)&gctx.pipelines);
  glGenFramebuffers(MAX_FBOS, (uint32*)&gctx.fbos);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(debugOutputCallback, 0);
}

uint32 gfxCreateVBO(void* data, uint32 size)
{
    glBindBuffer(GL_ARRAY_BUFFER, gctx.vbo[gctx.vboCnt]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return gctx.vbo[gctx.vboCnt++];
}

uint32 gfxCreateIBO(void* data, uint32 size)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gctx.ibo[gctx.iboCnt]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return gctx.ibo[gctx.iboCnt++];
}

typedef struct TextureFormatsGL {
  uint32 sizedInternalFormat;
  uint32 baseInternalFormat;
} TextureFormatsGL;

static const uint8 s_requestedComponents[TEX_Count] = 
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

static const TextureFormatsGL s_textureFormats[TEX_Count] =
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
    glTexStorage2D(GL_TEXTURE_2D, 1, format.sizedInternalFormat, w, h);
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

uint32 gfxCreateShader(const char* filename, uint8 shaderType) {
  char* shaderSrc;
  size_t srcSize;
  int ret = load_file(filename, &shaderSrc, &srcSize);

  ASSERT(ret == FILELOAD_SUCCESS);

  uint32 program = glCreateShaderProgramv(s_shaderTypes[shaderType], 1, (const char**)&shaderSrc);
  GLint isLinked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    char* log_str = malloc(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, log_str);
    printf("Program link failed: %s\n", log_str);
    free(log_str);
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

  *colorTex = color;
  *depthTex = depth;
  return fbo;
}

void gfxShutdown() {
  glDeleteBuffers(MAX_VBOS, (uint32*)&gctx.vbo);
  glDeleteBuffers(MAX_IBOS, (uint32*)&gctx.ibo);
  glDeleteTextures(MAX_TEXTURES, (uint32*)&gctx.tex);
  glDeleteProgramPipelines(MAX_PIPELINES, (uint32*)&gctx.pipelines);
  glDeleteFramebuffers(MAX_FBOS, (uint32*)&gctx.fbos);

  for (uint16 ii=0; ii<gctx.shaderCnt; ii++) {
    glDeleteProgram(gctx.programs[ii]);
  }
}

void gfxBindImage2D(uint32 image, uint32 img_unit, uint32 access, uint8 format) {
    glBindImageTexture(img_unit, image, 0, GL_FALSE, 0, access, s_textureFormats[format].sizedInternalFormat);
}
