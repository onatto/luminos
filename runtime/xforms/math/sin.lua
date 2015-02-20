xform sin
module math
dispname Sin

inputs
f32 rad = 0.0
f32 frequency = 3.0
f32 amplitude = 1.0
f32 offset = 0.0
f32 phaseShift = 0.0

outputs
f32 sin

func eval
out.sin = math.sin(inp.rad * inp.frequency + inp.phaseShift) * inp.amplitude + inp.offset
