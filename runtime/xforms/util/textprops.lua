module util

in u8 r = 255
in u8 g = 255
in u8 b = 255
in u8 a = 255
in str font = header
in u8 fontSize = 32

out textProps textProps
dispname Text Properties

xform textProps
out.textProps.r = inp.r
out.textProps.g = inp.g
out.textProps.b = inp.b
out.textProps.a = inp.a
out.textProps.font = inp.font
out.textProps.fontSize = inp.fontSize
