package.path = ";./scripts/?.lua"

--serverIP = "188.166.27.157"
serverIP = "localhost"
local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local lexer = require 'lexer'
local network = require 'network'

--serverIP = "ws://188.166.27.157:8126/luminos"
debugger.init()

local C = ffi.C

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

core_xforms, math_xforms, util_xforms = {}, {}, {}
CurrentNode = nil


local function loadTransforms()
    local modules = {"core", "math", "util"}
    local transforms = { {"mouse", "concat", "time"},
                         {"sin", "cos", "add", "multiply", "divide", "mod"},
                         {"print", "posws", "textprops"}
                        }
        for modidx,module in ipairs(modules) do
            for xformidx,transform in ipairs(transforms[modidx]) do
                lexer.lex(module,transform)
            end
        end

        for idx, node in pairs(core.nodes) do
            if node then
                node.xform = lexer.xform(node.module, node.submodule)
            end
        end
end

loadTransforms()

function portProgramInit()
    C.nw_send("Workspace")
end

function portProgramShutdown()
end

function portRenderNodes()
end

function portDisplayRuntimeError(error_msg)
    debugger.print("Error!")
    debugger.print(error_msg)
    local x, y = 2, 0
    C.uiSetTextProperties("header-bold", 25, 9)
    C.uiSetTextColor(255, 255, 255, 200)
    C.uiDrawText(x, y, g_statusMsg)
    C.uiSetTextProperties("header", 25, 9)
    y = y + 25
    if error_msg then
        C.uiDrawText(x, y, error_msg)
    end
    local traceback = debug.traceback()
    for line in traceback:gmatch("[^\r\n]+") do
        y = y + 25
        C.uiDrawText(x, y, line)
    end
end

function portProgramStart()
    core.programStart()

    local selected_nodes = ui.getSelectedNodes()
    if #selected_nodes > 0 then
        CurrentNode = selected_nodes[1]
    end

    if (ui.getKeyboardState(SDL.Key.F3) == KeyEvent.Press) then
        loadTransforms()
    end

    ui.start()
    ui.dragWorkspace()
    ui.drawNodes()
    ui.selectNodes()
    ui.dragNodes()
    ui.dragConnectors()
    ui.drawWorkspace()
    ui.update()

    ui.drawNodeInfo(CurrentNode)

    if CurrentNode then
        core.execNode(CurrentNode)
    end
end
