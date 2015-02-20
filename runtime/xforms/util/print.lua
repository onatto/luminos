xform print
module util
dispname Print String

inputs
f32 x = 0
f32 y = 0
textProps textProps
str text = Hello

func eval
local props = inp.textProps
if props then
    C.uiSetTextColor(props.r, props.g, props.b, props.a)
    C.uiSetTextProperties(props.font, math.floor(props.fontSize), 9)
end
C.uiDrawText(inp.x, inp.y, tostring(inp.text))

