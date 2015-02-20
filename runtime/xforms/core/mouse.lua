xform mouse
module core
dispname Mouse

outputs
u32 mx
u32 my

func eval
out.mx = g_mouseState.mx
out.my = g_mouseState.my
