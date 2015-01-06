module math

in f32 rad = 0.0
in f32 frequency = 1.0
in f32 amplitude = 1.0

out f32 cos

xform Cos
out.cos = math.cos(inp.rad * inp.frequency) * inp.amplitude
