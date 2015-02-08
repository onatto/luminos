module util

in f32 x = 0
in f32 y = 0
in textProps textProps
in str text = Hello

dispname Print String
xform Print
local props = inp.textProps
if props then
    C.uiSetTextColor(props.r, props.g, props.b, props.a)
    C.uiSetTextProperties(props.font, math.floor(props.fontSize), 9)
end
C.uiDrawText(inp.x, inp.y, tostring(inp.text))
