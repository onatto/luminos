-- UI Module

local ui = {}
local ffi = require 'ffi'
local debugger = require 'debugger'
local helpers = require 'helpers'
local core = require 'core'
local SDL =   require 'sdlkeys'
local bit = require 'bit'
local lexer = require 'lexer'

ffi.cdef
[[
    uint32_t uiDrawNode(float x, float y, float w, float h, uint8_t state, const char* title, uint8_t numInputs, uint8_t numOutputs, float time);
    void uiDrawPort(float x, float y, int widget_state, char r, char g, char b, char a);
    void uiDrawWire(float px, float py, float qx, float qy, int start_state, int end_state);
    uint8_t uiGetKeyboardState(uint16_t key);
    void uiWarpMouseInWindow(int x, int y);
    void uiSaveNVGState();
    void uiRestoreNVG();
    void uiSetTextProperties(const char* font, float size, int align);
    void uiSetTextColor(int r, int g, int b, int a);
    void uiDrawText(float x, float y, const char* str);
]]

local C = ffi.C

ui.drawNode = ffi.C.uiDrawNode
ui.drawPort = ffi.C.uiDrawPort
ui.drawWire = ffi.C.uiDrawWire
ui.getKeyboardState = ffi.C.uiGetKeyboardState
ui.warpMouse = ffi.C.uiWarpMouseInWindow

local BNDWidgetState = { Default = 0, Hover = 1, Active = 2 }

function ui.createNode(id, x, y, w, h, module, submodule)
    local node = {}
    node.sx = x
    node.sy = y
    node.w = w
    node.h = h
    node.id = id
    node.bndWidgetState = BNDWidgetState.Default
    node.constants = {}
    node.connections = {}
    node.module = module
    node.submodule = submodule
    node.xform = lexer.xform(module,submodule)
    node.input_values = {}
    node.output_values = {}

    -- Initialise table values to {} so that their contents can be set inside the xform
    for idx, output in ipairs(node.xform.outputs) do
       if (lexer.generaliseType(output.type) == lexer.Types.Table) then
          node.output_values[output.name] = {}
       end
    end

    core.nodes[id] = node
    --table.insert(core.nodes, node)
    return node
end

function ui.shutdown()
end

local zooming = {
    -- Center x,y
    cx = 0,
    cy = 0,
    zoom = 1,
    aspect = 1600 / 900
}

-- Holds data for dragging
local MouseDrag =
{
    mx = nil,       -- To calculate total delta vector
    my = nil,
    anchorx = {},  -- Starting(Anchor) point of the node before dragging, new_pos = anchor + delta
    anchory = {},
}
local SelectedNodes = {}

-- States
local MouseX, MouseY
local IHoldLCTRL
local IPressLMB, IHoldLMB, IReleaseLMB
local IPressRMB, IHoldRMB, IReleaseRMB
local DraggingNodes, DraggingConnectors, DraggingWorkspace
local IPressEnter
local IPressHome
local IPressTab
local IHoldLShift
local IPressEnter
local IPressEscape
local IPressBackspace
local IPressInsert
local IPressDelete
local IPressHome
local IPressTab
local IPressLShift
local IHoldPageUp
local IHoldPageDown

function ui.start()
    MouseX, MouseY = g_mouseState.mx, g_mouseState.my
    IPressLMB = g_mouseState.left == KeyEvent.Press
    IPressRMB = g_mouseState.right == KeyEvent.Press
    IReleaseRMB = g_mouseState.right == KeyEvent.Release
    IReleaseLMB = g_mouseState.left == KeyEvent.Release
    IHoldLMB = g_mouseState.left == KeyEvent.Hold
    IHoldRMB = g_mouseState.right == KeyEvent.Hold
    IPressEnter = ui.getKeyboardState(SDL.Key.RETURN) == KeyEvent.Press
    IPressHome = ui.getKeyboardState(SDL.Key.HOME) == KeyEvent.Press
    IPressTab  = ui.getKeyboardState(SDL.Key.TAB) == KeyEvent.Press
    IHoldLCTRL = ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold
    IHoldLShift  = ui.getKeyboardState(SDL.Key.LSHIFT) == KeyEvent.Hold
    IPressEscape = ui.getKeyboardState(SDL.Key.ESCAPE) == KeyEvent.Press
    IPressBackspace = ui.getKeyboardState(SDL.Key.BACKSPACE) == KeyEvent.Press
    IPressInsert = ui.getKeyboardState(SDL.Key.INSERT) == KeyEvent.Press
    IPressDelete = ui.getKeyboardState(SDL.Key.DELETE) == KeyEvent.Press
    IPressTab = ui.getKeyboardState(SDL.Key.TAB) == KeyEvent.Press
    IHoldPageUp = ui.getKeyboardState(SDL.Key.PAGEUP) == KeyEvent.Hold
    IHoldPageDown = ui.getKeyboardState(SDL.Key.PAGEDOWN) == KeyEvent.Hold
end

-- This is one of the most complex parts of the code
-- InputNode, OutputNode, HoveredInput, HoveredOutput are set by DrawNode function
-- Every connection is made from NodeStart->NodeEnd, NodeStart being the node that provides the input
-- to NodeEnd's output at PortStart and PortEnd respectively
-- So NodeStart is the cache for the node that we start dragging the connector from
local InputNode, OutputNode, HoveredInput, HoveredOutput
local PortStart, PortEnd, NodeStart, NodeEnd, NodeStartID, NodeEndID
local NodeStartIsInput
function ui.dragConnectors()
    local MouseOnPort = HoveredInput or HoveredOutput

    local StartDraggingConnector = function ()
        MouseDrag.mx = MouseX
        MouseDrag.my = MouseY
        MouseDrag.canchorx = HoveredPortX
        MouseDrag.canchory = HoveredPortY
        NodeStartID = (InputNode or OutputNode).id
        NodeStart = core.nodes[NodeStartID]
        PortStart = HoveredInput or HoveredOutput
        if NodeStart == InputNode then
           NodeStartIsInput = true
        else
           NodeStartIsInput = false
        end
        NodeEnd = nil
        NodeEndID = nil
        PortEnd = nil
        DraggingConnectors = true
    end
    local StopDraggingConnector = function ()
        DraggingConnectors = false
        NodeStart = nil
        NodeStartID = nil
        PortStart = nil
        NodeEnd = nil
        NodeEndID = nil
        PortEnd = nil
    end

    if DraggingConnectors then
       NodeStart = core.nodes[NodeStartID]
       NodeEnd   = core.nodes[NodeEndID]
    end

    local CreateConnection = function ()
       if not NodeStartIsInput then
          NodeStart, PortStart, NodeEnd, PortEnd, NodeStartID, NodeEndID = NodeEnd, PortEnd, NodeStart, PortStart, NodeEndID, NodeStartID
       end
       NodeStart.connections[NodeStart.xform.input_map[PortStart]] = {out_node_id = NodeEnd.id, port_name = NodeEnd.xform.output_map[PortEnd]}
       C.nw_send("UpdateConn " .. tostring(NodeStart.id) .. " " .. NodeStart.xform.input_map[PortStart] .. " " .. tostring(NodeEnd.id) .. " " .. NodeEnd.xform.output_map[PortEnd])
    end

    if IPressLMB and MouseOnPort then
       StartDraggingConnector()
       -- Delete existing connection if there is any
       connection = NodeStart.connections[NodeStart.xform.input_map[PortStart]]
       if NodeStartIsInput and connection then
          NodeStart.connections[NodeStart.xform.input_map[PortStart]] = nil
          C.nw_send("DeleteConn " .. tostring(NodeStart.id) .. " " .. NodeStart.xform.inputs[PortStart].name)
       end
    end

    if IHoldLMB and DraggingConnectors then
        -- Set NodeEnd if intersecting an Input/Output port
        if NodeStartIsInput and OutputNode then
           NodeEndID = OutputNode.id
           PortEnd = HoveredOutput
        elseif not NodeStartIsInput and InputNode then
           NodeEndID = InputNode.id
           PortEnd = HoveredInput
        end
        -- Draw the dragged wire
        ui.drawWire(MouseDrag.canchorx, MouseDrag.canchory, MouseX, MouseY, BNDWidgetState.Active, BNDWidgetState.Active)
    end

    if IReleaseLMB and DraggingConnectors then
        -- NodeStartIsInput and HoveredInput check is to not connect two inputs together
        -- NodeStartID and NodeEndID cannot be equal => Can't connect inputs/outputs of same node as it would introduce cycles
        if NodeStart and NodeEnd and not(NodeStartID == NodeEndID) and not (NodeStartIsInput and HoveredInput) then
            CreateConnection()
        end
        StopDraggingConnector()
    end
end

local function DrawNode(node)
    local numInputs = #node.xform.inputs
    local numOutputs = #node.xform.outputs
    nodeStatus = ui.drawNode(node.x, node.y, node.w, node.h, node.bndWidgetState, 
    node.xform.dispname, numInputs, numOutputs, g_time)

    local selectedInput = bit.rshift(nodeStatus, 16)
    local selectedOutput = bit.rshift(nodeStatus, 8)
    local nodeHovered = not(bit.lshift(nodeStatus, 31) == 0)

    -- Draw wires
    for inputName, connection in pairs(node.connections) do
       local inputNode = node -- Node providing the input
       local outputNode = core.nodes[connection.out_node_id]  -- Node providing the output to the input
       if outputNode then
           local outNodeOutputs = #outputNode.xform.outputs
           local inputPortIdx   = inputNode.xform.input_name_map[inputName]
           local outputPortIdx  = outputNode.xform.output_name_map[connection.port_name]
           local inputPortX  = inputNode.x + (inputPortIdx * inputNode.w / (numInputs+1))
           local inputPortY  = inputNode.y + inputNode.h
           local outputPortX = outputNode.x + (outputPortIdx * outputNode.w / (outNodeOutputs+1))
           local outputPortY = outputNode.y
           if inputPortX < outputPortX then
              inputPortX, outputPortX, inputPortY, outputPortY = outputPortX, inputPortX, outputPortY, inputPortY
           end
           ui.drawWire(inputPortX, inputPortY, outputPortX, outputPortY,
           BNDWidgetState.Active, BNDWidgetState.Active)
        end
     end

     if nodeHovered and selectedInput == 0 and selectedOutput == 0 then
        if not DraggingNodes then
           HoveredNode = node
        end
     end

     if selectedOutput > 0 then
        OutputNode = node
        HoveredOutput = selectedOutput
        HoveredPortX = node.x + (selectedOutput * node.w / (numOutputs+1))
        HoveredPortY = node.y
     end
     if selectedInput > 0 then
        InputNode = node
        HoveredInput = selectedInput
        HoveredPortX = node.x + (selectedInput * node.w / (numInputs+1))
        HoveredPortY = node.y + node.h
     end
end

function ui.drawNodes()
   HoveredNode = nil
   InputNode = nil
   OutputNode = nil
   HoveredInput = nil
   HoveredOutput = nil
   for idx, node in pairs(core.nodes) do
      if node then
         DrawNode(node)
      end
   end
end

local function SearchInSelectedNodes(snode)
    for i, node in ipairs(SelectedNodes) do
        if node == snode then
            return i
        end
    end
    return nil
end

function ui.selectNodes()
    -- This is the behaviour expected for holding CTRL, can override
    local SelectNodeCTRL = function ()
        FoundNode = SearchInSelectedNodes(HoveredNode)
        if FoundNode then
            local removedNode = table.remove(SelectedNodes, FoundNode)
            removedNode .bndWidgetState = BNDWidgetState.Default
        else
            table.insert(SelectedNodes, HoveredNode)
        end
    end

    local SelectNodeWithoutCTRL = function ()
       for i, node in ipairs(SelectedNodes) do
          node.bndWidgetState = BNDWidgetState.Default
       end
       SelectedNodes = {HoveredNode}
    end

    if IPressLMB and HoveredNode then
        if IHoldLCTRL then
            SelectNodeCTRL()
        elseif #SelectedNodes <= 1 then
            SelectNodeWithoutCTRL()
        end
    end

    if IPressLMB and not HoveredNode then
       for i, node in ipairs(SelectedNodes) do
          node.bndWidgetState = BNDWidgetState.Default
       end
        SelectedNodes = {}
    end

    for i, node in ipairs(SelectedNodes) do
        node.bndWidgetState = BNDWidgetState.Active
    end
end

function ui.dragNodes()
    -- States
    local HoveredNodeAlreadySelected = SearchInSelectedNodes(HoveredNode)
    local HoveringAPort = not not(PortStart or PortEnd)

    -- Functional blocks
    local StartDraggingNodes = function ()
        MouseDrag.mx = MouseX
        MouseDrag.my = MouseY
        DraggingNodes = true
        for i, node in ipairs(SelectedNodes) do
            MouseDrag.anchorx[i] = node.sx
            MouseDrag.anchory[i] = node.sy
        end
    end

    local DragDaNodes = function ()
        for i, node in ipairs(SelectedNodes) do
            node.sx = MouseDrag.anchorx[i] + (MouseX - MouseDrag.mx) / zooming.zoom
            node.sy = MouseDrag.anchory[i] + (MouseY - MouseDrag.my) / zooming.zoom
        end
    end

    local StopDraggingNodes = function ()
        for i, node in ipairs(SelectedNodes) do
            C.nw_send("UpdateNodePos " .. tostring(node.id) .. " " .. tostring(node.sx) .. " " .. tostring(node.sy))
        end
        DraggingNodes = false
    end

    -- Behavior description
    if IPressLMB and HoveredNodeAlreadySelected and not HoveringAPort then
        StartDraggingNodes()
    end

    if IHoldLMB and DraggingNodes then
        DragDaNodes()
    end

    if IReleaseLMB then
        StopDraggingNodes()
    end
end

function ui.getSelectedNodes()
    return SelectedNodes
end

function ui.drawWorkspace()
    local x, y = 2, 0

    C.uiSetTextProperties("header-bold", 25, 9)
    C.uiSetTextColor(255, 255, 255, 200)
    C.uiDrawText(x, y, g_statusMsg)
    C.uiSetTextProperties("header", 25, 9)
    y = y + 28

    if g_errorMsg then
        C.uiDrawText(x, y, g_errorMsg)
    end
end

local SelectedInput = 1

function ui.drawNodeInfo(node, y)
    if not CurrentNode then
        return
    end
    local w, h= 1600, 900
    local x, y = 2, 80
    local header_size = 22
    local param_size = 20
    local align = 1
    local input_cnt = #(CurrentNode.xform.inputs)
    local output_cnt = #(CurrentNode.xform.outputs)
    C.uiSetTextProperties("header-bold", header_size, align)
    C.uiSetTextColor(255, 255, 255, 50)
    C.uiDrawText(x, y, "Inputs")
    y = y + param_size

    C.uiSetTextColor(255, 255, 255, 255)
    for idx, input in pairs(node.xform.inputs) do
        local name = node.xform.input_map[idx]
        connection = node.connections[name]
        if idx == SelectedInput then
            C.uiSetTextColor(255, 150, 100, 255)
        else
            C.uiSetTextColor(255, 255, 255, 255)
        end
        C.uiSetTextProperties("header-bold", param_size, align)
        C.uiDrawText(x, y, name)
        y = y + param_size
        C.uiSetTextProperties("header", param_size - 2, align)
        if idx == SelectedInput and ui.UpdatingConstants then
           C.uiDrawText(x, y, ui.TextInput)
        else
           C.uiDrawText(x, y, lexer.convertToString(node.input_values[name], node.xform.inputs[idx].type))
        end
        y = y + param_size
    end

    C.uiSetTextProperties("header-bold", header_size, align)
    C.uiSetTextColor(255, 255, 255, 50)
    C.uiDrawText(x, y, "Outputs")
    y = y + param_size

    C.uiSetTextColor(255, 255, 255, 255)
    for idx, output in pairs(node.xform.outputs) do
        local name = node.xform.output_map[idx]
        C.uiSetTextProperties("header-bold", param_size, align)
        C.uiDrawText(x, y, name)
        y = y + param_size
        C.uiSetTextProperties("header", param_size - 2, align)
        C.uiDrawText(x, y, lexer.convertToString(node.output_values[name], node.xform.outputs[idx].type))
        y = y + param_size
    end
end

function ui.dragWorkspace()
    local ZoomIn = function ()
        zooming.zoom = zooming.zoom + 0.01
    end

    local ZoomOut = function ()
        zooming.zoom = zooming.zoom - 0.01
    end

    local StartDraggingWorkspace = function (CenterX, CenterY)
        DraggingWorkspace = true
        MouseDrag.mx = g_mouseState.mx
        MouseDrag.my = g_mouseState.my
        MouseDrag.wanchorx = CenterX
        MouseDrag.wanchory = CenterY
    end

    local DragWorkspace = function ()
        zooming.cx = MouseDrag.wanchorx + (MouseDrag.mx - g_mouseState.mx) / zooming.zoom
        zooming.cy = MouseDrag.wanchory + (MouseDrag.my - g_mouseState.my) / zooming.zoom
    end

    local StopDraggingWorkspace = function ()
        DraggingWorkspace = false
    end

    local UpdateNodePositions = function (CenterX, CenterY, ZoomAmount)
        for _k, node in pairs(core.nodes) do
           if node then
              node.x = (-CenterX + node.sx) * ZoomAmount
              node.y = (-CenterY + node.sy) * ZoomAmount
              node.w = 180 * ZoomAmount
              node.h = 40 * ZoomAmount
           end
        end
    end

    if IHoldPageUp then
        ZoomIn()
    elseif IHoldPageDown then
        ZoomOut()
    end

    local CenterX = zooming.cx
    local CenterY = zooming.cy
    local ZoomAmount = zooming.zoom -- [1,2]

    g_centerX = CenterX
    g_centerY = CenterY

    UpdateNodePositions(CenterX, CenterY, ZoomAmount)

    if IPressRMB and not (DraggingNodes or DraggingConnector) then
        StartDraggingWorkspace(CenterX, CenterY)
    end

    if IHoldRMB and DraggingWorkspace then
        DragWorkspace()
    end

    if IReleaseRMB then
        StopDraggingWorkspace()
    end
end

local EnteringCommands = false

ui.ReceivingTextInput = false
ui.TextInput = ""
ui.EnteringCommands = false
ui.UpdatingConstants = false

local function CreateNodeRequest(args)
    if not args or #args < 1 then
        return
    end
    local xform = args[1]
    cmp = helpers.split(xform, '/')
    local module, submodule = cmp[1], cmp[2]
    local xformTable = lexer.xform(module, submodule)
    if not xformTable then
       debugger.print("Couldn't create node with xform: " .. xform)
       return
    end

    req = "CreateNode " .. table.concat({g_mouseState.mx, g_mouseState.my, 180, 90,  xform, xformTable.name}, " ")
    if #args > 1 then
        req = req .. " " .. table.concat(args, " ")
    end
    C.nw_send(req)
end

local CmdMap = {
    cn = CreateNodeRequest
}


function ui.update()
   local EnteringCommands = ui.EnteringCommands
   local UpdatingConstants = ui.UpdatingConstants
   local ReceivingTextInput = ui.ReceivingTextInput
   local function StartTextInput()
      ui.ReceivingTextInput = true
   end
   local function StopTextInput()
       ui.ReceivingTextInput = false
       ui.TextInput = ""
   end

   local function StartEnteringCommands()
      ui.EnteringCommands = true
      StartTextInput()
   end
   local function StopEnteringCommands()
      ui.EnteringCommands = false
      StopTextInput()
   end
   local function StartUpdatingConstant()
      ui.UpdatingConstants = true
      local InputName = CurrentNode.xform.input_map[SelectedInput]
      ui.TextInput = tostring(CurrentNode.constants[InputName])
      StartTextInput()
   end
   local function StopUpdatingConstant()
      ui.UpdatingConstants = false
      StopTextInput()
   end
   local function DeleteNodesMsg(SelectedNodes)
      for idx, node in ipairs(SelectedNodes) do
         C.nw_send("DeleteNode " .. tostring(node.id))
      end
   end

   local function ReloadWorkspaceMsg()
      C.nw_send("Workspace")
   end
    local SelectNextInput = function (CurrentNode)
        SelectedInput = ((SelectedInput) % #(CurrentNode.xform.inputs)) + 1
    end
    local SelectPrevInput = function (CurrentNode)
        if SelectedInput == 1 then
            SelectedInput = #(CurrentNode.xform.inputs)
        else
            SelectedInput = SelectedInput - 1
        end
    end
    local function ProcessCmd()
       args = helpers.SplitWhitespace(ui.TextInput)
       cmd = args[1]
       table.remove(args, 1)
       if CmdMap[cmd] then
          CmdMap[cmd](args)
       end
       StopTextInput()
    end
   local function UpdateConstant()
      local InputName = CurrentNode.xform.input_map[SelectedInput]
      local NewConst = lexer.convertFromString(ui.TextInput, CurrentNode.xform.inputs[SelectedInput].type)
      -- Sanitize input at lexer.convertFromString
      if NewConst then
         CurrentNode.constants[InputName] = NewConst
         C.nw_send("UpdateConst " .. tostring(CurrentNode.id) .. " " .. InputName .. " " .. tostring(NewConst))
      end
   end
   local function SelectedInputIsTable()
      return lexer.generaliseType(CurrentNode.xform.inputs[SelectedInput].type) == lexer.Types.Table
   end
   -- Logic starts here
   if IPressEnter and not ReceivingTextInput then
      StartEnteringCommands()
   elseif IPressEnter and EnteringCommands then
      ProcessCmd()
      StopEnteringCommands()
   end

   if IPressInsert and not ReceivingTextInput and not SelectedInputIsTable() then
      StartUpdatingConstant()
   end

   if IPressEnter and UpdatingConstants then
      UpdateConstant()
      StopUpdatingConstant()
   end

    if IPressEscape then
       if EnteringCommands then
          StopEnteringCommands()
       elseif UpdatingConstants then
          StopUpdatingConstant()
       end
    end

    if IPressBackspace and ReceivingTextInput then
       ui.TextInput = string.sub(ui.TextInput, 1, -2)
    end

    if IPressBackspace and not ReceivingTextInput and not SelectedInputIsTable() then
       if not IHoldLShift then
          StartUpdatingConstant()
          ui.TextInput = ""
       else
         local InputName = CurrentNode.xform.input_map[SelectedInput]
         CurrentNode.constants[InputName] = CurrentNode.xform.inputs[SelectedInput].default
         C.nw_send("DeleteConst " .. tostring(CurrentNode.id) .. " " .. InputName)
       end
    end
    if EnteringCommands then
        C.uiDrawText(0, 30, "!cmd: " .. ui.TextInput)
    end

    if IPressDelete and SelectedNodes then
       DeleteNodesMsg(SelectedNodes)
    end
    if IPressHome then
       ReloadWorkspaceMsg()
    end

    if IPressTab then
        if IHoldLShift then
            SelectPrevInput(CurrentNode)
        else
            SelectNextInput(CurrentNode)
        end
    end
end

function portTextEdit(text)
    if ui.ReceivingTextInput then
       ui.TextInput = ui.TextInput .. text
    end
end

return ui
