#pragma once

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

void gfxInit();
void gfxShutdown();

FFI_EXPORT uint32 gfxCreateVBO(void* data, uint32 size);
FFI_EXPORT uint32 gfxCreateIBO(void* data, uint32 size);
FFI_EXPORT uint32 gfxCreateUBO(uint32 size);
FFI_EXPORT uint32 gfxCreateTexture2D(const char* filename, uint16* w, uint16* h, uint8 texFormat, uint8 numMips);
FFI_EXPORT uint32 gfxCreateImage2D(uint16 w, uint16 h, uint8 texFormat);
FFI_EXPORT uint32 gfxCreateShader(const char* filename, uint8 shaderType);
FFI_EXPORT uint32 gfxCreateShaderSource(const char* src, uint8 shaderType);
FFI_EXPORT uint32 gfxCreatePipeline();
FFI_EXPORT uint32 gfxCreateFramebuffer(uint16 width, uint16 height, uint8 colorFormat, uint8 depthFormat, uint32* colorTex, uint32* depthTex);

FFI_EXPORT void gfxBindVertexBuffer(uint32 vbo, uint8 bindingPoint, uint8 stride);
FFI_EXPORT void gfxBindIndexBuffer(uint32 ibo);
FFI_EXPORT void gfxVertexFormat(uint8 vertexFormat);
FFI_EXPORT void gfxBindFramebuffer(uint32 fbo);
FFI_EXPORT void gfxResizeTexture(uint32 tex, uint8 format, uint32 width, uint32 height);
FFI_EXPORT void gfxBindTextures2D(uint32* texs, int8* locations, uint8 numTextures, uint32 program); 
FFI_EXPORT void gfxBindImage2D(uint32 image, uint32 img_unit, uint8 format);
FFI_EXPORT void gfxSetImageUnit(uint32 program, uint32 imageLocation, int32 imgUnit);
FFI_EXPORT void gfxBindSSQuadBuffers();
FFI_EXPORT void gfxUniformBindingPoint(uint32 shader, const char* uniformBlockName, uint32 bindingPoint);
FFI_EXPORT void gfxBindUniformBuffer(uint32 ubo, void* data, size_t size, uint32 bindingPoint);

FFI_EXPORT void gfxBindPipeline(uint32 pipeline);
FFI_EXPORT void gfxDispatchCompute(uint32 x, uint32 y, uint32 z);
FFI_EXPORT void gfxReplaceGeomShader(uint32 pipeline, uint32 geom);
FFI_EXPORT void gfxReplaceFragShader(uint32 pipeline, uint32 frag);
FFI_EXPORT void gfxReplaceVertexShader(uint32 pipeline, uint32 vert);
FFI_EXPORT void gfxReplaceShaders(uint32 pipeline, uint32 vert, uint32 frag);
FFI_EXPORT void gfxReplaceShadersGeom(uint32 pipeline, uint32 vert, uint32 frag, uint32 geom);
FFI_EXPORT void gfxReplaceComputeShader(uint32 pipeline, uint32 comp);
FFI_EXPORT int32 gfxGetShaderUniforms(uint32 program, uint8* namesBuffer, uint32 namesBufferSize, uint8* types, uint8* locations);

FFI_EXPORT void gfxSetTransform(float* transform);
FFI_EXPORT void gfxBlitTexture(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h);
FFI_EXPORT void gfxBlitFramebuffer(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h);

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
  TEXTURE_FORMATS
};

enum ShaderTypes {
  SHADER_VERT,
  SHADER_FRAG,
  SHADER_COMP,
  SHADER_GEOM,
  SHADER_COUNT
};

enum VertexFormats {
  VERT_POS_NOR,
  VERT_POS_NOR_STRIDED,
  VERT_POS_T0_STRIDED,
  VERT_POS_NOR_T0,
  VERT_POS_NOR_T0_STRIDED,
  VERTEX_FORMATS,
};

enum UniformTypes {
  UNIFORM_FLOAT,
  UNIFORM_VEC2,
  UNIFORM_VEC3,
  UNIFORM_VEC4,
  UNIFORM_MAT2,
  UNIFORM_MAT3,
  UNIFORM_MAT4,
  UNIFORM_SAMPLER1D,
  UNIFORM_SAMPLER2D,
  UNIFORM_SAMPLER3D,
  UNIFORM_SAMPLER_CUBE,
  UNIFORM_IMAGE1D,
  UNIFORM_IMAGE2D,
  UNIFORM_IMAGE2D_ARRAY,
  UNIFORM_IMAGE3D,
  UNIFORM_TYPES,
};
