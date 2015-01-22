module math

in f32 rad = 0.0
in f32 frequency = 3.0
in f32 amplitude = 1.0
in f32 offset = 0.0
in f32 phaseShift = 0.0

out f32 sin

xform Sin
out.sin = math.sin(inp.rad * inp.frequency + inp.phaseShift) * inp.amplitude + inp.offset
