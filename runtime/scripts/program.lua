package.path = ";./scripts/?.lua"

serverIP = "localhost"
--serverIP = "localhost"
local ffi =   require 'ffi'
local core =  require "core"
local ui =    require 'ui'
local SDL =   require 'sdlkeys'
local debugger = require 'debugger'
local lexer = require 'lexer'
local network = require 'network'
local viz   = require 'viz'

debugger.init()

local C = ffi.C

-- We needn't even pass tables around, if we are storing those tables(xforms) in an array
-- We just pass that transforms index in that array
-- This is stored in the core module

core_xforms, math_xforms, util_xforms = {}, {}, {}
CurrentNode = nil


local function loadTransforms()
    local modules = {"core", "math", "util", "gfx"}
    local transforms = { {"mouse", "concat", "time"},
                         {"sin", "cos", "add", "multiply", "divide", "mod"},
                         {"print", "posws", "textprops"},
                         {"loadtex2d", "image2d", "dispatchcs"}
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
    local x, y = 2, g_windowHeight - 200
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

    if (ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold and ui.getKeyboardState(SDL.Key.L) == KeyEvent.Press) then
        C.coreNewWorkspace()
        C.coreLoadWorkspace()
        C.coreSetupWorkspace()
    end
    if (ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold and ui.getKeyboardState(SDL.Key.S) == KeyEvent.Press) then
        core.saveWorkspace()
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
    ui.xformList()

    if CurrentNode then
        local vizFunc = lexer.xformFunc[CurrentNode.xform.module][CurrentNode.xform.name].viz
        if vizFunc then
            vizFunc(CurrentNode.input_values, CurrentNode.output_values)
        end
        if (ui.getKeyboardState(SDL.Key.F2) == KeyEvent.Press) then
            core.cacheNode(CurrentNode)
        end
        core.execNode(CurrentNode)
    end

    viz.showVisualisations()
end
