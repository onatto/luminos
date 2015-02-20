xform textprops
module util
dispname Text Properties

inputs
u8 r = 255
u8 g = 255
u8 b = 255
u8 a = 255
str font = header
u8 fontSize = 32

outputs
textProps textProps

func eval
out.textProps.r = inp.r
out.textProps.g = inp.g
out.textProps.b = inp.b
out.textProps.a = inp.a
out.textProps.font = inp.font
out.textProps.fontSize = inp.fontSize
