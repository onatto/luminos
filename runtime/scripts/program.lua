package.path = ";./scripts/?.lua"

local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local commands = require 'commands'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local util = require 'util_xforms'
local helpers = require 'helpers'
debugger.init()

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

local workspace = {}
workspace.current_node = nil
workspace.util_port_node = nil

function workspace.save()
    workspace.ui_nodes = ui.nodes
    helpers.saveTable(workspace, "workspace.lua")
end
function workspace.load()
    local ws, err = helpers.loadTable("workspace.lua")
    if ws then
        workspace = ws
        ui.nodes = workspace.ui_nodes
    elseif err then
        debugger.print(err)
        return false
    end
end

function portProgramInit()
    workspace.current_node = ui.createNode(250, 100, core.concat_xform, {str_a = "Time is: "})
    workspace.util_port_node = ui.createNode(400, 100, 'util.inport')
    ui.createNode(200, 400, 'core.mouse_xform')
    ui.createNode(400, 300, 'core.time_xform')
    ui.createNode(100, 300, 'util.print_xform')
end

function portProgramShutdown()
    ui.shutdown()
end

function portProgramStart()
    local transforms = ui.getTransforms()
    for _k,transform in ipairs(transforms) do
        transform.visited = false
    end

    core.execNode(workspace.current_node)
    core.execNode(workspace.util_port_node)

    if (ui.getKeyboardState(SDL.Key.F2) == KeyEvent.Press) then
        debugger.printTable(current_node)
    end

    if g_mouseState.middle == KeyEvent.Press then
        ui.createNode(g_mouseState.mx, g_mouseState.my, core.concat_xform)
    end

    if (ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold and ui.getKeyboardState(SDL.Key.S) == KeyEvent.Press) then
        workspace.save()
    end

    if (ui.getKeyboardState(SDL.Key.F6) == KeyEvent.Press) then
        workspace.load()
    end

    ui.drawNodes()
    ui.dragNodes()
    ui.dragConnectors()

    local idx = 0
    for name, out in pairs(workspace.current_node.xform.outputs) do
        ui.dbgText(4 + idx, name .. " : " .. tostring(out.value))
        idx = idx + 1
    end
end

