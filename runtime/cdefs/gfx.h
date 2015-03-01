typedef int32_t int32;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef int8_t int8;

void gfxInit();
void gfxShutdown();

uint32 gfxCreateVBO(void* data, uint32 size);
uint32 gfxCreateIBO(void* data, uint32 size);
uint32 gfxCreateUBO(uint32 size);
uint32 gfxCreateTexture2D(const char* filename, uint16* w, uint16* h, uint8 texFormat, uint8 numMips);
uint32 gfxCreateImage2D(uint16 w, uint16 h, uint8 texFormat);
void gfxSetImageUnit(uint32 program, uint32 imageLocation, int32 imgUnit);
uint32 gfxCreateShader(const char* filename, uint8 shaderType);
uint32 gfxCreateShaderSource(const char* src, uint8 shaderType);
uint32 gfxCreatePipeline();
uint32 gfxCreateFramebuffer(uint16 width, uint16 height, uint8 colorFormat, uint8 depthFormat, uint32* colorTex, uint32* depthTex);

void gfxBindVertexBuffer(uint32 vbo, uint8 bindingPoint, uint8 stride);
void gfxBindIndexBuffer(uint32 ibo);
void gfxVertexFormat(uint8 vertexFormat);
void gfxBindFramebuffer(uint32 fbo);
void gfxResizeTexture(uint32 tex, uint8 format, uint32 width, uint32 height);
void gfxBindTextures2D(uint32* texs, int8* locations, uint8 numTextures, uint32 program); 
void gfxBindImage2D(uint32 image, uint32 img_unit, uint8 format);
void gfxBindSSQuadBuffers();
void gfxUniformBindingPoint(uint32 shader, const char* uniformBlockName, uint32 bindingPoint);
void gfxBindUniformBuffer(uint32 ubo, void* data, size_t size, uint32 bindingPoint);

void gfxBindPipeline(uint32 pipeline);
void gfxReplaceGeomShader(uint32 pipeline, uint32 geom);
void gfxReplaceFragShader(uint32 pipeline, uint32 frag);
void gfxReplaceVertexShader(uint32 pipeline, uint32 vert);
void gfxReplaceShaders(uint32 pipeline, uint32 vert, uint32 frag);
void gfxReplaceShadersGeom(uint32 pipeline, uint32 vert, uint32 frag, uint32 geom);
void gfxReplaceComputeShader(uint32 pipeline, uint32 comp);
int32 gfxGetShaderUniforms(uint32 program, uint8* namesBuffer, uint32 namesBufferSize, uint8* types, uint8* locations);

void gfxSetTransform(float* transform);
void gfxBlitTexture(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h);
void gfxBlitFramebuffer(uint32 tex, float x, float y, float w, float h, float wnd_w, float wnd_h);

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
