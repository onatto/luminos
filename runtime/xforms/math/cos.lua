xform cos
module math
dispname Cos

inputs
f32 rad = 0.0
f32 frequency = 1.0
f32 amplitude = 1.0
f32 offset = 0.0
f32 phaseShift = 0.0

outputs
f32 cos

func eval
out.cos = math.cos(inp.rad * inp.frequency + inp.phaseShift) * inp.amplitude + inp.offset
