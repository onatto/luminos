package.path = ";./scripts/?.lua"

local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local commands = require 'commands'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local util = require 'util_xforms'
debugger.init()

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

current_node = nil
util_port_node = nil
function portProgramInit()
    current_node = ui.createNode(350, 100, "core.concat_xform", {str_a = "Time is: "})
    util_port_node = ui.createNode(400, 100, "util_xforms.inport")
    ui.createNode(200, 400, "core.mouse_xform")
    ui.createNode(400, 300, "core.time_xform")
    ui.createNode(100, 300, "util_xforms.print_xform")
end

function portProgramShutdown()
    ui.shutdown()
end

function portProgramStart()
    local transforms = ui.getTransforms()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end

    local selected_nodes = ui.getSelectedNodes()
    if #selected_nodes > 0 then
        current_node = selected_nodes[1]
    end

    core.execNode(current_node)
    core.execNode(util_port_node)

    if (ui.getKeyboardState(SDL.Key.F2) == KeyEvent.Press) then
        debugger.printTable(current_node)
    end

    if g_mouseState.middle == KeyEvent.Press then
        ui.createNode(g_mouseState.mx, g_mouseState.my, core.concat_xform)
    end

    if (ui.getKeyboardState(SDL.Key.F6) == KeyEvent.Press) then
        commands.compile("scripts/ui.lua", status, err)
        ui = require('ui')
    end

    ui.drawNodes()
    ui.selectNodes()
    ui.dragNodes()
    ui.dragConnectors()

    ui.drawNodeInfo(current_node)
end

