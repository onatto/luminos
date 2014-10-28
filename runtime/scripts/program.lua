package.path = ";./scripts/?.lua"

core =  require "core"
ui =    require "ui"
xform = require "xform_ops"
commands = require "commands"
SDL =   require "sdlkeys"
debugger = require "debugger"
debugger.init()

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module
local concat_0 = xform.clone(core.concat_xform, {str_a = "Time: "})
local concat_1 = xform.clone(core.concat_xform, {str_a = "MouseX: " })

local mouse_xform = xform.clone(core.mouse_xform, {})
local time_xform = xform.clone(core.time_xform, {})

concat_0.connections.str_b = {transform = time_xform, name = "time"}
concat_1.connections.str_b = {transform = mouse_xform, name = "mx"}

ui.createNode(400, 0, concat_0)
ui.createNode(200, 300, mouse_xform)

local top_transform = {
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
   ui.dbgText(base_y, "mx: " .. g_mouseState.mx)
   ui.dbgText(base_y+1, "my: " .. g_mouseState.my)
   ui.dbgText(base_y+2, "Left: " .. KeyEventEnum[g_mouseState.left+1])
   ui.dbgText(base_y+3, "Right: " .. KeyEventEnum[g_mouseState.right+1])
   ui.dbgText(base_y+4, "Middle: " .. KeyEventEnum[g_mouseState.middle+1])
end


function portProgramStart()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end
    core.execTransform(top_transform)
    if (ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold) then
        if (ui.getKeyboardState(SDL.Key.A) == KeyEvent.Press) then
            counter = counter+1
        end
    end

    if (g_mouseState.left == KeyEvent.Release) then
        counter = counter+1
    end

    ui.drawNodes()
    ui.dragNodes()

    stdout = top_transform.outputs.stdout.value

    ui.dbgText(0, status_msg)
    ui.dbgText(1, error_msg)
    ui.dbgText(2, stdout)
    dbgMouseData(4)
    ui.dbgText(9, tostring(counter))
    return stdout
end

