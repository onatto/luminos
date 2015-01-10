package.path = ";./scripts/?.lua"

local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local lexer = require 'lexer'
local network = require 'network'

serverIP = "ws://188.166.27.157:8126/luminos"
debugger.init()

local C = ffi.C

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

core_xforms, math_xforms, util_xforms = {}, {}, {}
current_node = nil

local function loadTransforms()
    lexer.lex("core", "mouse")
    lexer.lex("core", "concat")
    lexer.lex("core", "time")
    lexer.lex("math", "sin")
    lexer.lex("util", "print")
end

loadTransforms()

function portProgramInit()
    debugger.printTable(core_xforms.Concat_transform)
    --current_node = ui.createNode(350, 100, 180, 40, "core", "concat", {str_left = "Time is: "})
    --ui.createNode(200, 400, 180, 40, "core", "mouse")
    --ui.createNode(400, 300, 180, 40, "core", "time")
    --ui.createNode(100, 300, 180, 40, "util", "print")
    --ui.createNode(100, 400, 180, 40, "math", "sin")
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

    ui.KeyboardControls(selected_nodes)

    if current_node then
        core.execNode(current_node)
    end

    if (ui.getKeyboardState(SDL.Key.F3) == KeyEvent.Press) then
        loadTransforms()
    end

    if (ui.getKeyboardState(SDL.Key.C) == KeyEvent.Press) then
        C.nw_send("Workspace")
        --C.nw_send("CreateNode 100 200 150 90 sin Sin")
        --C.nw_send("LockNode 1 onat 5")
        --C.nw_send("UnlockNode 1 onat")
    end
    
    ui.start()
    ui.dragWorkspace()
    ui.SelectNodes()
    ui.DragNodes()
    ui.DragConnectors()
    ui.drawNodes()
    ui.drawWorkspace()
    ui.Proceed(current_node)
    ui.EditText()

    ui.drawNodeInfo(current_node)
end
