xform posws
module util
dispname Workspace Position

inputs
f32 offsetX = 0
f32 offsetY = 0

outputs
f32 x
f32 y

func eval
out.x = inp.offsetX - g_centerX
out.y = inp.offsetY - g_centerY

-- g_centerXY is defined/set in ui.DragWorkspace
