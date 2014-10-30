package.path = ";./scripts/?.lua"

local ffi =   require 'ffi'
local core =  require "core"
local ui =    require "ui"
local commands = require "commands"
local SDL =   require "sdlkeys"
local debugger = require "debugger"
debugger.init()

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

current_node = nil
function portProgramInit()
    current_node = ui.createNode(350, 100, core.concat_xform, {str_a = "Time is: "})
    ui.createNode(200, 300, core.mouse_xform)
    ui.createNode(500, 300, core.time_xform)
end

function portProgramShutdown()
    ui.shutdown()
end

function portProgramStart()
    local transforms = ui.getTransforms()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end

    core.execNode(current_node)
    ui.dbgText(3, "hey")
    if (ui.getKeyboardState(SDL.Key.F2) == KeyEvent.Press) then
        debugger.printTable(current_node)
    end

    if (ui.getKeyboardState(SDL.Key.F6) == KeyEvent.Press) then
        commands.compile("scripts/ui.lua", status, err)
        ui = require('ui')
    end

    ui.drawNodes()
    ui.dragNodes()
    ui.dragConnectors()

    local idx = 0
    for name, out in pairs(current_node.xform.outputs) do
        ui.dbgText(4 + idx, name .. " : " .. out.value)
        idx = idx + 1
    end
end

