package.path = ";./scripts/?.lua"

ffi =   require 'ffi'
core =  require "core"
ui =    require "ui"
commands = require "commands"
SDL =   require "sdlkeys"
debugger = require "debugger"
debugger.init()

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

function portProgramInit()
    ui.createNode(400, 0, core.concat_xform, {str_a = "Time is: "})
    ui.createNode(200, 300, core.mouse_xform)

    top_transform = {
        name = "stdout",
        inputs = {
            input_str = {type = "string", default = ""},
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
end

function portProgramShutdown()
    ui.shutdown()
end

local status = ffi.new("uint8_t[?]", 1024)
local err = ffi.new("uint8_t[?]", 1024)
function portProgramStart()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end
    core.execNode(top_transform)
    if (ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold) then
        if (ui.getKeyboardState(SDL.Key.A) == KeyEvent.Press) then
            counter = counter+1
        end
    end

    if (ui.getKeyboardState(SDL.Key.F6) == KeyEvent.Press) then
        commands.compile("scripts/ui.lua", status, err)
        ui = require('ui')
    end

    if (g_mouseState.left == KeyEvent.Release) then
        counter = counter+1
    end

    ui.drawNodes()
    ui.dragNodes()
    ui.dragConnectors()

    ui.dbgText(0, status)
    ui.dbgText(1, err)
    ui.dbgText(2, stdout)
    ui.dbgText(9, tostring(counter))
end

