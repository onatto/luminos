#include "types.h"
#include "gl44.h"
#define OVR_OS_WIN32
#include <../Src/OVR_CAPI_GL.h>

#include "gfx.h"

static ovrHmd hmd = 0;
static ovrEyeRenderDesc eyesDesc[2];
static ovrPosef eyePoses[2];
static ovrFrameTiming timing;
static ovrTexture eyeTextures[2];
uint32 renderTarget, rtWidth, rtHeight, rtColorTex;
static ovrSizei rtSize;
bool vrInit()
{
   ovrBool success = ovr_Initialize();
   return (bool)success;
}

void vrCreateHMD(void* wnd)
{
   ovrBool result;
   hmd = ovrHmd_Create(0);
   if (hmd == NULL) {
	  hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
   }

   // Direct back buffer output from the window to HMD
   //result = ovrHmd_AttachToWindow(hmd, wnd, NULL, NULL);

   // Configure OpenGL.
   union ovrGLConfig cfg;
   cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
   cfg.OGL.Header.BackBufferSize.w = hmd->Resolution.w;
   cfg.OGL.Header.BackBufferSize.h = hmd->Resolution.h;
   cfg.OGL.Header.Multisample = 1;
   cfg.OGL.Window = (HWND)wnd;

   ovrFovPort eyesFov[2];
   eyesFov[0] = hmd->DefaultEyeFov[0];
   eyesFov[1] = hmd->DefaultEyeFov[1];

   ovrHmd_SetEnabledCaps(hmd
         , 0
//       | ovrHmdCap_LowPersistence
         | ovrHmdCap_DynamicPrediction
         );

   ovrHmd_ConfigureTracking(hmd
         , 0
         | ovrTrackingCap_Orientation
         | ovrTrackingCap_MagYawCorrection
         | ovrTrackingCap_Position
         , 0
         );

   result = ovrHmd_ConfigureRendering(hmd
         , &cfg.Config
         , 0
         | ovrDistortionCap_Chromatic
         | ovrDistortionCap_Vignette
         | ovrDistortionCap_TimeWarp
         | ovrDistortionCap_Overdrive
         | ovrDistortionCap_NoRestore
         , eyesFov
         , eyesDesc // out to eyeDesc
         );

   ovrSizei sizeL = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,  hmd->DefaultEyeFov[0], 1.0f);
   ovrSizei sizeR = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);
   rtWidth = sizeL.w + sizeR.w;
   rtHeight = (sizeL.h>sizeR.h) ? sizeL.h : sizeR.h; //max(sizeL.h, sizeR.h);
   rtSize.w = rtWidth;
   rtSize.h = rtHeight;

   renderTarget = gfxCreateFramebuffer(rtWidth, rtHeight, TEX_RGBA8, TEX_D24F, &rtColorTex, 0);
   uint32 tex = gfxCreateTexture2D("textures/aaa.png", 0, 0, TEX_RGBA8, 0);
   gfxBindFramebuffer(renderTarget);
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   gfxBlitTexture(tex, 0.f, 0.f, 1280.f, 800.f, (float)rtWidth, (float)rtHeight);
   gfxBindFramebuffer(0);
}

void vrBeginFrame()
{
   ovrGLTexture texture;
   texture.OGL.Header.API         = ovrRenderAPI_OpenGL;
   texture.OGL.Header.TextureSize = rtSize;
   texture.OGL.TexId              = rtColorTex;

   ovrTexture _texture = texture.Texture;

   eyePoses[0] = ovrHmd_GetHmdPosePerEye(hmd, ovrEye_Left);
   eyePoses[1] = ovrHmd_GetHmdPosePerEye(hmd, ovrEye_Right);

   eyeTextures[0] = _texture;
   eyeTextures[1] = _texture;

   ovrRecti rect;
   rect.Pos.x  = 0;
   rect.Pos.y  = 0;
   rect.Size.w = rtWidth/2;
   rect.Size.h = rtHeight;

   eyeTextures[0].Header.RenderViewport = rect;

   rect.Pos.x += rect.Size.w;
   eyeTextures[1].Header.RenderViewport = rect;

   timing = ovrHmd_BeginFrame(hmd, 0);
}
void vrEndFrame()
{
   ovrHmd_EndFrame(hmd, eyePoses, eyeTextures);
}

void vrShutdown()
{
   ovrHmd_Destroy(hmd);
   ovr_Shutdown();
}
