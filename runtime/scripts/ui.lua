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
    void ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
    void ui_drawPort(float x, float y, int widget_state, char r, char g, char b, char a);
    void ui_drawWire(float px, float py, float qx, float qy, int start_state, int end_state);
    void ui_dbgTextPrintf(int y, const char *str);
    uint8_t ui_getKeyboardState(uint16_t key);
    void ui_warpMouseInWindow(int x, int y);
    void ui_saveNVGState();
    void ui_restoreNVG();
    void ui_setTextProperties(const char* font, float size, int align);
    void ui_setTextColor(int r, int g, int b, int a);
    void ui_drawText(float x, float y, const char* str);
]]

local C = ffi.C

ui.drawNode = ffi.C.ui_drawNode
ui.drawPort = ffi.C.ui_drawPort
ui.dbgText = ffi.C.ui_dbgTextPrintf
ui.drawWire = ffi.C.ui_drawWire
ui.getKeyboardState = ffi.C.ui_getKeyboardState
ui.warpMouse = ffi.C.ui_warpMouseInWindow

local BNDWidgetState = { Default = 0, Hover = 1, Active = 2 }

function ui.initPorts(node)
    node.ports = {}
    -- Calculate input port locations
    local i = 1
    local input_cnt = #(node.xform.inputs)
    for input_idx, input in ipairs(node.xform.inputs) do
        local port = { name = input.name, type = input.type }
        port.x = (1/(input_cnt+1)) * i
        port.y = 0.85
        port.bndWidgetState = BNDWidgetState.Default
        port.is_input = true
        port.is_output = false
        node.ports[input.name] = port
        i = i+1
    end

    -- Calculate output port locations
    i = 1
    local output_cnt = #(node.xform.outputs)
    for output_idx, output in ipairs(node.xform.outputs) do
        local port = { name = output.name, type = output.type}
        port.x = (1/(output_cnt+1)) * i
        port.y = 0.2
        port.bndWidgetState = BNDWidgetState.Default
        port.is_input = false
        port.is_output = true
        node.ports[output.name] = port
        i = i+1
        
        -- Initialize output tables to {}
        if lexer.generaliseType(output.type) == lexer.Types.Table then
           node.output_values[output.name] = {}
        end
    end
end

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
    ui.initPorts(node)

    core.nodes[id] = node
    --table.insert(core.nodes, node)
    return node
end

function ui.shutdown()
end

local function DrawNode(node)
    ui.drawNode(node.x, node.y, node.w, node.h, node.bndWidgetState, node.xform.dispname, 255, 50, 100, 255)
    for name, port in pairs(node.ports) do
        if port.is_input then
            ui.drawPort(node.x + port.x * node.w, node.y + port.y * node.h, port.bndWidgetState, 0, 100, 255, 255)
        else
            ui.drawPort(node.x + port.x * node.w, node.y + port.y * node.h, port.bndWidgetState, 0, 255, 100, 255)
        end
    end
    -- Draw the input connections
    for inport_name, connection in pairs(node.connections) do
        local node_in = node
        local node_out = core.nodes[connection.out_node_id]
        if node_out then
           local port_in = node.ports[inport_name]
           local port_out = node_out.ports[connection.port_name]
           local node_inx = node_in.x + port_in.x * node_in.w
           local node_iny = node_in.y + port_in.y * node_in.h
           local node_outx = node_out.x + port_out.x * node_out.w
           local node_outy = node_out.y + port_out.y * node_out.h
           if node_inx < node_outx then
              ui.drawWire(node_inx,
              node_iny,
              node_outx,
              node_outy,
              BNDWidgetState.Active, BNDWidgetState.Active)
           else
              ui.drawWire(node_outx,
              node_outy,
              node_inx,
              node_iny,
              BNDWidgetState.Active, BNDWidgetState.Active)
           end
        end
    end
end

local function max(a,b)
    if a > b then
        return a
    end
    return b
end

local zooming = {
    -- Center x,y
    cx = 0,
    cy = 0,
    zoom = 1,
    aspect = 1600 / 900
}
function ui.drawNodes()
    for idx, node in pairs(core.nodes) do
       if node then
          DrawNode(node)
       end
    end
end

local function pt_pt_dist2(px, py, qx, qy)
    local dx = px-qx
    local dy = py-qy
    return dx*dx + dy*dy
end
local function pt_aabb_test(minx, miny, w, h, px, py)
    if minx < px and px < minx + w and miny < py and py < miny + h then
        return true
    end
    return false
end
local function pt_aabb_relative(minx, miny, w, h, px, py)
    return (px - minx) / w, (py - miny) / h
end

local function ports_pt_intersect(node, px, py)
    local isect = nil
    local radius = 0.006
    for _k, port in pairs(node.ports) do
        local dist2 = pt_pt_dist2(px, py, port.x, port.y)
        if dist2 < radius then
            port.bndWidgetState = BNDWidgetState.Hover
            isect = port
        else
            port.bndWidgetState = BNDWidgetState.Default
        end
    end
    return isect
end

local function nodes_pt_intersect(px, py)
    local isect = nil
    for _k, node in pairs(core.nodes) do
       if node then
          insideAABB = pt_aabb_test(node.x, node.y, node.w, node.h, px, py)
          if insideAABB and not isect then
             node.bndWidgetState = BNDWidgetState.Hover
             isect = node
          else
             node.bndWidgetState = BNDWidgetState.Default
          end
       end
    end
    return isect
end

-- Holds data for dragging
local MouseDrag =
{
    mx = nil,       -- To calculate total delta vector
    my = nil,
    anchorx = {},  -- Starting(Anchor) point of the node before dragging, new_pos = anchor + delta
    anchory = {},
}
local HoveredNode
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

local InputNode, OutputNode, InputPort, OutputPort
local PortStart, PortEnd, NodeStart, NodeEnd

function ui.dragConnectors()
    local MouseOnPort = PortStart

    local Types = { Float = 0, Integer = 1, String = 2, VecN = 3, Table = 4}
    local GeneraliseType = function(Type)
        if Type == 'f16' or Type == 'f32' or Type == 'f64' then
            return Types.Float
        elseif Type == 'i8' or Type == 'i16' or Type == 'i32' or Type == 'i64' or  Type == 'u8' or Type == 'u16' or Type == 'u32' or Type == 'u64' then
            return Types.Float
        elseif Type == 'str' then
            return Types.String
        elseif Type == 'vec2' or Type == 'vec3' or Type == 'vec4' then
            return Types.VecN
        else
            return Types.Table
        end
    end
    local PortTypesMatch = function (TypeA, TypeB)
        local GenA = GeneraliseType(TypeA)
        local GenB = GeneraliseType(TypeB)
        if GenA == Types.Other or GenB == Types.Other then
            return TypeA == TypeB
        elseif GenA == GenB then
            return true
        else
            return false
        end
    end

    local FindConnection = function ()
        local relx, rely = pt_aabb_relative(HoveredNode.x, HoveredNode.y, HoveredNode.w, HoveredNode.h, MouseX, MouseY)
        -- not drag_connector ==> node_from == nil
        -- drag_connector ==> node_from ~= nil
        if not HoveredNode then
            return
        end

        -- If not dragging connectors, cache the node/port tuple
        if not DraggingConnectors then
            NodeStart = HoveredNode
            PortStart = ports_pt_intersect(HoveredNode, relx, rely)
            if PortStart and PortStart.is_input then
                InputNode = NodeStart
                InputPort = PortStart
            end
        else
            NodeEnd = HoveredNode
            PortEnd = ports_pt_intersect(HoveredNode, relx, rely)
            if InputPort then
                OutputNode = NodeEnd
                OutputPort = PortEnd
            else
                OutputNode = NodeStart
                OutputPort = PortStart
                InputNode = NodeEnd
                InputPort = PortEnd
            end
        end
    end

    local StartDraggingConnector = function ()
        MouseDrag.mx = MouseX
        MouseDrag.my = MouseY
        MouseDrag.canchorx = NodeStart.x + NodeStart.w * PortStart.x
        MouseDrag.canchory = NodeStart.y + NodeStart.h * PortStart.y
        DraggingConnectors = true
    end
    local StopDraggingConnector = function ()
        DraggingConnectors = false
        DraggingExistingConnection = false
        NodeStart = nil
        PortStart = nil
        NodeEnd = nil
        PortEnd = nil
    end
    local DragExistingConnection = function ()
        -- If it is an input port and a binding exists
        if NodeStart.connections[PortStart.name] then
            DraggingExistingConnection = true
            -- Then, it is as if we're dragging from that OutputNode's output
            OutputNode = core.nodes[InputNode.connections[InputPort.name].out_node_id]
            OutputPort = OutputNode.ports[InputNode.connections[InputPort.name].port_name]
            -- New NodeStart is the input NodeStart's connection node
            NodeStart = OutputNode
            PortStart = OutputPort
            InputNode.connections[InputPort.name] = nil
            C.nw_send("DeleteConn " .. tostring(InputNode.id) .. " " .. InputPort.name)
        end
    end
    local CreateConnection = function ()
        if not (NodeStart == NodeEnd) and not (PortStart.is_input == PortEnd.is_input) then
            -- Lets connect those ports = Make the xform input/output connections
            InputNode, OutputNode = NodeStart, NodeEnd
            InputPort, OutputPort = PortStart, PortEnd
            -- Swap inputs if PortEnd is an input
            if PortEnd.is_input then
                InputNode, OutputNode = NodeEnd, NodeStart
                InputPort, OutputPort = PortEnd, PortStart
            end
            --if PortTypesMatch(InputPort.type, OutputPort.type) then
                -- Connect the port that is an input of a node to the output port
                InputNode.connections[InputPort.name] = {out_node_id = OutputNode.id, port_name = OutputPort.name}
                C.nw_send("UpdateConn " .. InputNode.id .. " " .. InputPort.name .. " " .. tostring(OutputNode.id) .. " " .. OutputPort.name)
            --end
        end
    end

    if not DraggingNodes and HoveredNode then
        FindConnection()
    end
    if IPressLMB and MouseOnPort and HoveredNode then
        DragExistingConnection()
        StartDraggingConnector()
    end

    if IHoldLMB and DraggingConnectors then
        if MouseDrag.canchorx < MouseX then
            ui.drawWire(MouseDrag.canchorx, MouseDrag.canchory, MouseX, MouseY, BNDWidgetState.Active, BNDWidgetState.Active)
        else
            ui.drawWire(MouseX, MouseY, MouseDrag.canchorx, MouseDrag.canchory, BNDWidgetState.Active, BNDWidgetState.Active)
        end
    end

    if IReleaseLMB and DraggingConnectors then
        if NodeEnd and PortEnd then
            CreateConnection()
        end
        StopDraggingConnector()
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

local function FindHoveredNode(MouseX, MouseY)
    return nodes_pt_intersect(MouseX, MouseY)
end

function ui.selectNodes()
    -- This is the behaviour expected for holding CTRL, can override
    local SelectNodeCTRL = function ()
        FoundNode = SearchInSelectedNodes(HoveredNode)
        if FoundNode then
            table.remove(SelectedNodes, FoundNode)
        else
            table.insert(SelectedNodes, HoveredNode)
        end
    end

    local SelectNodeWithoutCTRL = function ()
        FoundNode = SearchInSelectedNodes(HoveredNode)
        if not FoundNode then
            SelectedNodes = {HoveredNode}
        end
    end

    if not DraggingNodes then
        HoveredNode = FindHoveredNode(MouseX, MouseY)
    end

    if IPressLMB and HoveredNode then
        if IHoldLCTRL then
            SelectNodeCTRL()
        elseif #SelectedNodes <= 1 then
            SelectNodeWithoutCTRL()
        end
    end

    if IPressLMB and not HoveredNode then
        SelectedNodes = {}
    end

    -- This behavior is for handling the CTRL
    if IReleaseLMB and HoveredNode then
        SelectNodeWithoutCTRL()
    end

    for i, node in ipairs(SelectedNodes) do
        node.bndWidgetState = BNDWidgetState.Active
    end
    return SelectedNodes
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
            node.bndWidgetState = BNDWidgetState.Active
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

    C.ui_setTextProperties("header-bold", 25, 9)
    C.ui_setTextColor(255, 255, 255, 200)
    C.ui_drawText(x, y, g_statusMsg)
    C.ui_setTextProperties("header", 25, 9)
    y = y + 28

    if g_errorMsg then
        C.ui_drawText(x, y, g_errorMsg)
    end
end

local SelectedInput = 1

function ui.drawNodeInfo(node, y)
    if not CurrentNode then
        return
    end
    local w, h= 1600, 900
    local x, y = 2, 80
    local header_size = 18
    local param_size = 16
    local align = 1
    local input_cnt = #(CurrentNode.xform.inputs)
    local output_cnt = #(CurrentNode.xform.outputs)
    C.ui_setTextProperties("header-bold", header_size, align)
    C.ui_setTextColor(255, 255, 255, 50)
    C.ui_drawText(x, y, "Inputs")
    y = y + param_size

    C.ui_setTextColor(255, 255, 255, 255)
    for idx, input in pairs(node.xform.inputs) do
        local name = node.xform.input_map[idx]
        connection = node.connections[name]
        if idx == SelectedInput then
            C.ui_setTextColor(255, 150, 100, 255)
        else
            C.ui_setTextColor(255, 255, 255, 255)
        end
        C.ui_setTextProperties("header-bold", param_size, align)
        C.ui_drawText(x, y, name)
        y = y + param_size
        C.ui_setTextProperties("header", param_size - 2, align)
        if idx == SelectedInput and ui.UpdatingConstants then
           C.ui_drawText(x, y, ui.TextInput)
        else
           C.ui_drawText(x, y, lexer.convertToString(node.input_values[name], node.xform.inputs[idx].type))
        end
        y = y + param_size
    end

    C.ui_setTextProperties("header-bold", header_size, align)
    C.ui_setTextColor(255, 255, 255, 50)
    C.ui_drawText(x, y, "Outputs")
    y = y + param_size

    C.ui_setTextColor(255, 255, 255, 255)
    for idx, output in pairs(node.xform.outputs) do
        local name = node.xform.output_map[idx]
        C.ui_setTextProperties("header-bold", param_size, align)
        C.ui_drawText(x, y, name)
        y = y + param_size
        C.ui_setTextProperties("header", param_size - 2, align)
        C.ui_drawText(x, y, lexer.convertToString(node.output_values[name], node.xform.outputs[idx].type))
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
              C.ui_setTextProperties("header", 25, 9)
              C.ui_drawText(100 + 25*node.id, 400, tostring(node.id));
              if node.sy then
                 C.ui_drawText(100 + 25*node.id, 420, "Y");
              end
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
        C.ui_drawText(0, 30, "!cmd: " .. ui.TextInput)
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
