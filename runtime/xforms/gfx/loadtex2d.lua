xform loadtex2d
module gfx
dispname Load Texture2D
import gfx

inputs
str path
i8 numMips = 0
u8 texFormat = 10

outputs
texture tex

func cache
if #inp.path > 0 and not out.tex.hnd then
    local w,h
    out.tex.hnd = C.gfxCreateTexture2D(inp.path, w, h, inp.texFormat, inp.numMips)
    out.tex.w = w
    out.tex.h = h
end
func viz
if out.tex.hnd then
    C.gfxBlitTexture(out.tex.hnd, 0, 500, 300, 300, 1920, 1080)
end

func eval
out.tex = out.tex
