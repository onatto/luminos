xform mod
module math
dispname Mod

inputs
f32 num = 1
f32 dem = 1

outputs
f32 rest

func eval
out.rest = inp.num % inp.dem
