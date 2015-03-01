xform rendertarget
module gfx
dispname Render Target
import gfx

inputs
u16 width = 0
u16 height = 0
u8 colorFormat = 10
u8 depthFormat = 15

outputs
u32 colorTex
u32 depthTex
u32 fbo

func cache
if inp.width > 0 and inp.height > 0 then
    if out.fbo then
        if out.colorTex then
            C.gfxResizeTexture(out.colorTex, inp.colorFormat, inp.width, inp.height)
        end
        if out.depthTex then
            C.gfxResizeTexture(out.depthTex, inp.colorFormat, inp.width, inp.height)
        end
    else
        out.fbo = C.gfxCreateFramebuffer(inp.width, inp.height, inp.colorFormat, inp.depthFormat, out.colorTex, out.depthTex)
    end
end

func eval
if out.fbo then
    C.gfxBindFramebuffer(fbo);
end
