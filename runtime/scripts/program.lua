package.path = ";./scripts/?.lua"

local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local lexer = require 'lexer'

debugger.init()

local C = ffi.C

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

core_xforms, math_xforms, util_xforms = {}, {}, {}
current_node = nil

lexer.lex("core", "mouse")
lexer.lex("core", "concat")
lexer.lex("core", "time")
lexer.lex("math", "sin")
lexer.lex("util", "print")

function portProgramInit()
    debugger.printTable(core_xforms.Concat_transform)
    current_node = ui.createNode(350, 100, core_xforms.Concat_transform, {str_left = "Time is: "})
    ui.createNode(200, 400, core_xforms.Mouse_transform)
    ui.createNode(400, 300, core_xforms.Time_transform)
    ui.createNode(100, 300, util_xforms.Print_transform)
    ui.createNode(100, 400, math_xforms.Sin_transform)
end

function portProgramShutdown()
end

function portDisplayRuntimeError(error_msg)
    if not error_msg then
        return
    end
    local x, y = 2, 0
    C.ui_setTextProperties("header-bold", 25, 9)
    C.ui_setTextColor(255, 255, 255, 200)
    C.ui_drawText(x, y, g_statusMsg)
    C.ui_setTextProperties("header", 25, 9)
    y = y + 25
    C.ui_drawText(x, y, error_msg)
    y = y + 25
    C.ui_drawText(x, y, debug.traceback())
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

    if (ui.getKeyboardState(SDL.Key.F2) == KeyEvent.Press) then
        debugger.printTable(current_node)
    end

    ui.start()
    ui.dragWorkspace()
    ui.SelectNodes()
    ui.DragNodes()
    ui.DragConnectors()
    ui.drawNodes()
    ui.drawWorkspace()
    ui.Proceed(current_node)

    ui.drawNodeInfo(current_node)
end
