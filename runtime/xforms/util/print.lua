module util

in f32 x = 0
in f32 y = 0
in i8 fontSize = 16
in str text = Hello

dispname Print String
xform Print
C.ui_setTextProperties("header", math.floor(inp.fontSize), 9)
C.ui_drawText(inp.x, inp.y, tostring(inp.text))
