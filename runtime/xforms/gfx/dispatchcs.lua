xform dispatchcs
module gfx
dispname Dispatch CS
import gfx

inputs
u16 x = 256
u16 y = 256
u16 z = 1
u32 cs = 0

outputs
u32 pipe

func cache
if not out.pipe then
    out.pipe = C.gfxCreatePipeline()
end
C.gfxReplaceComputeShader(out.pipe, inp.cs)

func eval
if out.pipe then
    C.gfxBindPipeline(out.pipe)
    C.gfxDispatchCompute(inp.x, inp.y, inp.z)
end
