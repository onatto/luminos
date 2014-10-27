dofile "scripts/core.lua"
dofile "scripts/table_ops.lua"
dofile "scripts/xform_ops.lua"
dofile "scripts/ui_controls.lua"
dofile "scripts/sdlkeys.lua"
dofile "scripts/commands.lua"

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
concat_0 = clone_xform(concat_transform, {str_a = "Time: "})
concat_1 = clone_xform(concat_transform, {str_a = "MouseX: " })

concat_0.connections.str_b = {transform = time_xform, name = "time"}
concat_1.connections.str_b = {transform = mouse_xform, name = "mx"}

create_node(400, 0, concat_0)
create_node(200, 300, concat_1)

-- clone_xform(concat_transform, { str_a = "Say", str_b = " Hey" })
-- can set name field later from the UI, but gets the concat_transform's name

top_transform = {
    name = "stdout",
    inputs = {
        input_str = {type = "string", default = ""},
    },
    connections = {
      input_str = { transform = concat_0, name = "concat_str" }
    },
    outputs = {
        stdout = {type = "string"}
    },
    eval = function(self)
         self.outputs.stdout.value = self.inputs.input_str.value
    end
}

-- This table contains all the xforms present in the workspace
transforms = { concat_0, concat_1, top_transform, mouse_xform, time_xform}
counter = 0
error_msg = ""
status_msg = ""
stdout = ""

function dbgMouseData(base_y)
   dbgText(base_y, "mx: " .. g_mouseState.mx)
   dbgText(base_y+1, "my: " .. g_mouseState.my)
   dbgText(base_y+2, "Left: " .. KeyEventEnum[g_mouseState.left+1])
   dbgText(base_y+3, "Right: " .. KeyEventEnum[g_mouseState.right+1])
   dbgText(base_y+4, "Middle: " .. KeyEventEnum[g_mouseState.middle+1])
end

mouse_drag = {
    -- Starting positions for drag and node
    mx = nil,
    my = nil,
    nodex = nil,
    nodey = nil,
    dragging = false
}
selected_node = nil
function portProgramStart()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end
    execTransform(top_transform)
    if (getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold) then
        if (getKeyboardState(SDL.Key.A) == KeyEvent.Press) then
            counter = counter+1
        end
    end

    if (g_mouseState.left == KeyEvent.Release) then
        counter = counter+1
    end

    draw_nodes()

    if (g_mouseState.left == KeyEvent.Press) then
        selected_node = nodes_pt_intersect(g_mouseState.mx, g_mouseState.my)
        if (selected_node) then
            mouse_drag.mx = g_mouseState.mx
            mouse_drag.my = g_mouseState.my
            mouse_drag.nodex = selected_node.x
            mouse_drag.nodey = selected_node.y
            mouse_drag.dragging = true
        end
    end

    if (g_mouseState.left == KeyEvent.Hold and mouse_drag.dragging) then
        selected_node.x = mouse_drag.nodex + g_mouseState.mx - mouse_drag.mx
        selected_node.y = mouse_drag.nodey + g_mouseState.my - mouse_drag.my
    end

    if (g_mouseState.left == KeyEvent.Release and mouse_drag.dragging) then
        mouse_drag.dragging = false
    end

    stdout = top_transform.outputs.stdout.value

    dbgText(0, status_msg)
    dbgText(1, error_msg)
    dbgText(2, stdout)
    dbgMouseData(4)
    dbgText(9, tostring(counter))
    return stdout
end

