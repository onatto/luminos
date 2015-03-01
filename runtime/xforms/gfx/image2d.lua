xform image2d
module gfx
dispname Image2D
import gfx

inputs
u16 width = 256
u16 height = 256
u8 texFormat = 12

outputs
img2D img

func cache
if not out.img.hnd then
    out.img.hnd = C.gfxCreateImage2D(inp.width, inp.height, inp.texFormat)
    out.img.format = inp.texFormat
end

import viz
func viz
viz.set {
    hnd = out.img.hnd,
    viz = viz.texture2D,
}

func eval
out.img = out.img
