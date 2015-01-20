module util

in f32 x = 0
in f32 y = 0
in str text = Hello

dispname Print String
xform Print
C.ui_drawText(inp.x, inp.y, tostring(inp.text))
