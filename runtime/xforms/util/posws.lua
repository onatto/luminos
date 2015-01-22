module util

in f32 offsetX = 0
in f32 offsetY = 0

out f32 x
out f32 y

dispname Workspace Position
xform PosWorkspace
out.x = inp.offsetX - g_centerX
out.y = inp.offsetY - g_centerY

-- g_centerXY is defined/set in ui.DragWorkspace
