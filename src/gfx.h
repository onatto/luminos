#pragma once

void gfxInit();
void gfxShutdown();
uint32 gfxCreateVBO(void* data, uint32 size);
uint32 gfxCreateIBO(void* data, uint32 size);
uint32 gfxCreateTexture2D(const char* filename, uint16* w, uint16* h, uint8 texFormat, uint8 numMips);
uint32 gfxCreateImage2D(uint16 w, uint16 h, uint8 texFormat);
uint32 gfxCreateShader(const char* filename, uint8 shaderType);
uint32 gfxCreatePipeline();
void gfxBindImage2D(uint32 image, uint32 img_unit, uint32 access, uint8 format);
void gfxBindPipeline(uint32 pipeline);
void gfxReplaceGeomShader(uint32 pipeline, uint32 geom);
void gfxReplaceFragShader(uint32 pipeline, uint32 frag);
void gfxReplaceVertexShader(uint32 pipeline, uint32 vert);
void gfxReplaceShaders(uint32 pipeline, uint32 vert, uint32 frag);
void gfxReplaceShadersGeom(uint32 pipeline, uint32 vert, uint32 frag, uint32 geom);
void gfxReplaceComputeShader(uint32 pipeline, uint32 comp);

enum TextureFormats {
  TEX_R8,
  TEX_R16,
  TEX_R16F,
  TEX_R32,
  TEX_R32F,
  TEX_RG8,
  TEX_RG16,
  TEX_RG16F,
  TEX_RG32,
  TEX_RG32F,
  TEX_RGBA8,
  TEX_RGBA16,
  TEX_RGBA16F,
  TEX_RGBA32,
  TEX_RGBA32F,
  TEX_D16F,
  TEX_D24F,
  TEX_D32F,
  TEX_D24S8,
  TEX_Count
};

enum ShaderTypes {
  SHADER_VERT,
  SHADER_FRAG,
  SHADER_COMP,
  SHADER_GEOM,
  SHADER_COUNT
};