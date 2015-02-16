module math

in f32 rad = 0.0
in f32 frequency = 1.0
in f32 amplitude = 1.0
in f32 offset = 0.0
in f32 phaseShift = 0.0

out f32 cos
dispname Cos

xform Cos
out.cos = math.cos(inp.rad * inp.frequency + inp.phaseShift) * inp.amplitude + inp.offset
