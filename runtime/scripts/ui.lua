-- UI Module

local ui = {}
local ffi = require 'ffi'
local debugger = require 'debugger'
local helpers = require 'helpers'
local core = require 'core'
local SDL =   require 'sdlkeys'

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

local ui_nodes = {}
function ui.getTransforms()
    local transforms = {}
    for _i, node in ipairs(ui_nodes) do
        table.insert(transforms, node.xform)
    end
    return transforms
end

local function getTransform(xform_name)
    local start, ends = string.find(xform_name, '[.]')
    local module_name, xform_name = string.sub(xform_name, 1, start-1), string.sub(xform_name, ends+1)
    local module = require(module_name)
    local xform = module[xform_name]
    return xform
end

function ui.createNode(x, y, xform_name, constant_inputs)
    local node = {}
    node.sx = x
    node.sy = y
    node.x = x
    node.y = y
    node.w = 180
    node.h = 40
    node.bndWidgetState = BNDWidgetState.Default
    node.constants = {}
    node.connections = {}
    node.ports = {}
    node.xform_name = xform_name
    node.xform = core.cloneTransform(node, getTransform(xform_name))
    if constant_inputs then
        for input_name, constant in pairs(constant_inputs) do
            node.constants[input_name] = constant
        end
    end

    -- Calculate input port locations
    local i = 1
    local input_cnt = helpers.tableLength(node.xform.inputs)
    for input_name, input in pairs(node.xform.inputs) do
        local port = { name = input_name }
        port.x = (1/(input_cnt+1)) * i
        port.y = 0.85
        port.bndWidgetState = BNDWidgetState.Default
        port.is_input = true
        port.is_output = false
        node.ports[input_name] = port
        i = i+1
    end

    -- Calculate output port locations
    i = 1
    local output_cnt = helpers.tableLength(node.xform.outputs)
    for output_name, output in pairs(node.xform.outputs) do
        local port = { name = output_name }
        port.x = (1/(output_cnt+1)) * i
        port.y = 0.2
        port.bndWidgetState = BNDWidgetState.Default
        port.is_input = false
        port.is_output = true
        node.ports[output_name] = port
        i = i+1
    end

    table.insert(ui_nodes, node)
    return node
end

function ui.shutdown()
    ui_nodes = {}
end

local function drawNode(node)
    ui.drawNode(node.x, node.y, node.w, node.h, node.bndWidgetState, node.xform.name, 255, 50, 100, 255)
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
        local node_out = connection.node
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
    -- Wiggle
    -- for _k, node in pairs(ui_nodes) do
        -- node.w = (math.sin(g_time * 2) + 1.0) * zooming.aspect * 40 + 160
        -- node.h = (math.sin(g_time * 2) + 1.0) / zooming.aspect * 10 + 60
    -- end
    local cx = zooming.cx
    local cy = zooming.cy
    local zoom = zooming.zoom -- [1,2]

    for _k, node in pairs(ui_nodes) do
        node.x = (-cx + node.sx) * zoom
        node.y = (-cy + node.sy) * zoom
        node.w = 180 * zoom
        node.h = 40 * zoom
    end
    for _k, node in pairs(ui_nodes) do
        drawNode(node)
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
    for _k, node in pairs(ui_nodes) do
        insideAABB = pt_aabb_test(node.x, node.y, node.w, node.h, px, py)
        if insideAABB and not isect then
            node.bndWidgetState = BNDWidgetState.Hover
            isect = node
        else
            node.bndWidgetState = BNDWidgetState.Default
        end
    end
    return isect
end

-- Holds data for dragging
local mouse_drag =
{
    mx = nil,       -- To calculate total delta vector
    my = nil,
    anchorx = {},  -- Starting(Anchor) point of the node before dragging, new_pos = anchor + delta
    anchory = {},
}
local hovered_node = nil
local selected_nodes = {}
local node_from = nil
local node_to = nil
local port_from = nil
local port_to = nil
-- Dragging: port_from -> port_to
function ui.dragConnectors()
    local mx, my = g_mouseState.mx, g_mouseState.my
    -- If not dragging a node already and mouse is on a node, calculate relative position of the mouse in the nodes AABB
    -- Use the relative position to see if we're intersecting any ports
    if not mouse_drag.drag_node and hovered_node then
        local relx, rely = pt_aabb_relative(hovered_node.x, hovered_node.y, hovered_node.w, hovered_node.h, mx, my)
        -- not drag_connector ==> node_from == nil
        -- drag_connector ==> node_from ~= nil
        if not mouse_drag.drag_connector and hovered_node then
            node_from = hovered_node
            port_from = ports_pt_intersect(hovered_node, relx, rely)
        elseif mouse_drag.drag_connector and hovered_node then
            node_to = hovered_node
            port_to = ports_pt_intersect(hovered_node, relx, rely)
        end
    end

    -- On LMB Press, if mouse was on a port, start dragging that connector
    if g_mouseState.left == KeyEvent.Press then
        if port_from and not node_from.xform.is_hub then
            -- If it is an input port and a binding exists
            if node_from.connections[port_from.name] then
                -- Then, it is as if we're dragging from that output_node's output
                local input_node = node_from
                local input_port = port_from
                local output_node = input_node.connections[input_port.name].node
                local output_port = output_node.ports[input_node.connections[input_port.name].port_name]
                -- New node_from is the input node_from's connection node
                node_from = output_node
                port_from = output_port
                input_node.connections[input_port.name] = nil
            end
            mouse_drag.mx = mx
            mouse_drag.my = my
            mouse_drag.canchorx = node_from.x + node_from.w * port_from.x
            mouse_drag.canchory = node_from.y + node_from.h * port_from.y
            mouse_drag.drag_connector = true
        elseif port_from and node_from.xform.is_hub then
            node_from.connections[port_from.name] = nil
            local input_cnt = helpers.tableLength(node_from.connections)
            node_from.ports = {}
            node_from.xform.inputs = {}
            local i = 1
            local new_connections = {}
            for _i, input in pairs(node_from.connections) do
                new_connections[tostring(i)] = input
                i = i + 1
            end
            node_from.connections = helpers.shallowCopy(new_connections)
            i = 1
            for _i, connection in pairs(node_from.connections) do
                node_from.xform.inputs[tostring(i)] = {default=nil}
                local port = { name = tostring(i)}
                port.x = (1/(input_cnt+1)) * i
                port.y = 0.85
                port.bndWidgetState = BNDWidgetState.Default
                port.is_input = true
                port.is_output = false
                node_from.ports[tostring(i)] = port
                i = i + 1
            end
        end
    end

    -- If dragging, draw the wire when holding LMB
    if g_mouseState.left == KeyEvent.Hold and mouse_drag.drag_connector then
        ui.drawWire(mouse_drag.canchorx, mouse_drag.canchory, mx, my, BNDWidgetState.Active, BNDWidgetState.Active)
    end

    -- On LMB release, check if
    if g_mouseState.left == KeyEvent.Release then
        if mouse_drag.drag_connector and port_to and node_to then
            if not (node_from == node_to) and not (port_from.is_input == port_to.is_input) then
                -- Lets connect those ports = Make the xform input/output connections
                local input_node, output_node = node_from, node_to
                local input_port, output_port = port_from, port_to
                -- Swap inputs if port_to is an input
                if port_to.is_input then
                    input_node, output_node = node_to, node_from
                    input_port, output_port = port_to, port_from
                end
                -- Connect the port that is an input of a node to the output port
                input_node.connections[input_port.name] = {node = output_node, port_name = output_port.name}
                -- At this point input_node and output_node is there
            end
        elseif mouse_drag.drag_connector and node_to and (not port_to) and node_to.xform.is_hub then
            local input_node, output_node = node_to, node_from
            local output_port = port_from
            local input_cnt = helpers.tableLength(input_node.connections) + 1
            input_node.connections[tostring(input_cnt)] = {node = output_node, port_name = output_port.name}

            input_node.xform.inputs = {}
            -- Calculate input port locations
            for i, input in pairs(input_node.connections) do
                input_node.xform.inputs[tostring(i)] = {default=nil, type = "nil"}
                local port = { name = tostring(i)}
                port.x = (1/(input_cnt+1)) * i
                port.y = 0.85
                port.bndWidgetState = BNDWidgetState.Default
                port.is_input = true
                port.is_output = false
                input_node.ports[i] = port
            end
        end
        mouse_drag.drag_connector = false
        node_from = nil
        port_from = nil
        node_to = nil
        port_to = nil
    end
end

local function findNode(snode)
    for i, node in ipairs(selected_nodes) do
        if node == snode then
            return i
        end
    end
    return nil
end
function ui.selectNodes()
    local mx, my = g_mouseState.mx, g_mouseState.my

    -- If not dragging a node already, find a node to drag
    if not mouse_drag.drag_node then
        hovered_node = nodes_pt_intersect(mx, my)
    end

    if g_mouseState.left == KeyEvent.Press then
        if hovered_node then
            if ui.getKeyboardState(SDL.Key.LCTRL) == KeyEvent.Hold then
                found_node = findNode(hovered_node)
                if found_node then
                    table.remove(selected_nodes, found_node)
                else
                    table.insert(selected_nodes, hovered_node)
                end
            elseif #selected_nodes <= 1 then
                selected_nodes = {hovered_node}
            end
        else
            selected_nodes = {}
        end
    end
    if g_mouseState.left == KeyEvent.Release and hovered_node then
        found_node = findNode(hovered_node)
        if not found_node then
            selected_nodes = {hovered_node}
        end
    end

    for i, node in ipairs(selected_nodes) do
        node.bndWidgetState = BNDWidgetState.Active
    end
    return selected_nodes
end

function ui.dragNodes()
    local mx, my = g_mouseState.mx, g_mouseState.my
    if g_mouseState.left == KeyEvent.Press and findNode(hovered_node) then
        if #selected_nodes > 0 and not (port_from or port_to) then
            mouse_drag.mx = mx
            mouse_drag.my = my
            mouse_drag.drag_node = true
            for i, node in ipairs(selected_nodes) do
                mouse_drag.anchorx[i] = node.sx
                mouse_drag.anchory[i] = node.sy
            end
        end
    end

    -- Dragging a node if holding left mouse and we're dragging nodes
    if (g_mouseState.left == KeyEvent.Hold and mouse_drag.drag_node) then
        for i, node in ipairs(selected_nodes) do
            node.sx = mouse_drag.anchorx[i] + (mx - mouse_drag.mx) / zooming.zoom
            node.sy = mouse_drag.anchory[i] + (my - mouse_drag.my) / zooming.zoom
            node.bndWidgetState = BNDWidgetState.Active
        end
    end

    -- On LMB release, stop dragging nodes
    if g_mouseState.left == KeyEvent.Release then
        if mouse_drag.drag_node then
            mouse_drag.drag_node = false
        end
    end
end

function ui.getSelectedNodes()
    return selected_nodes
end

function ui.drawNodeInfo(node, y)
    local w, h= 1600, 900
    local x, y = 0, 120
    local header_size = 30
    local param_size = 22
    local align = 1
    local input_cnt = helpers.tableLength(current_node.xform.inputs)
    local output_cnt = helpers.tableLength(current_node.xform.outputs)
    C.ui_setTextProperties("header", header_size, align)
    C.ui_setTextColor(255, 255, 255, 50)
    C.ui_drawText(x, y, "Inputs")
    y = y + header_size

    C.ui_setTextProperties("header", param_size, align)
    C.ui_setTextColor(255, 255, 255, 255)
    for name, input in pairs(node.xform.inputs) do
        connection = node.connections[name]
        C.ui_drawText(x, y, name)
        y = y + param_size
        if not connection then
            C.ui_drawText(x, y, tostring(node.constants[name]))
        else
            C.ui_drawText(x, y, tostring(connection.node.xform.outputs[connection.port_name].value))
        end
        y = y + param_size
    end

    C.ui_setTextProperties("header", header_size, align)
    C.ui_setTextColor(255, 255, 255, 50)
    C.ui_drawText(x, y, "Outputs")
    y = y + header_size

    C.ui_setTextProperties("header", param_size, align)
    C.ui_setTextColor(255, 255, 255, 255)
    for name, input in pairs(node.xform.outputs) do
        C.ui_drawText(x, y, name)
        y = y + param_size
        C.ui_drawText(x, y, tostring(input.value))
        y = y + param_size
    end
end

function ui.dragWorkspace()
    if ui.getKeyboardState(SDL.Key.PAGEUP) == KeyEvent.Press then
        zooming.zoom = zooming.zoom + 0.2
    elseif ui.getKeyboardState(SDL.Key.PAGEDOWN) == KeyEvent.Press then
        zooming.zoom = zooming.zoom - 0.2
    end

    if g_mouseState.right == KeyEvent.Press and not (mouse_drag.drag_node or mouse_drag.drag_connector) then
        mouse_drag.drag_workspace = true
        mouse_drag.mx = g_mouseState.mx
        mouse_drag.my = g_mouseState.my
        mouse_drag.wanchorx = zooming.cx
        mouse_drag.wanchory = zooming.cy
    end

    if g_mouseState.right == KeyEvent.Hold and mouse_drag.drag_workspace then
        zooming.cx = mouse_drag.wanchorx + (mouse_drag.mx - g_mouseState.mx) / zooming.zoom
        zooming.cy = mouse_drag.wanchory + (mouse_drag.my - g_mouseState.my) / zooming.zoom
    end

    if g_mouseState.right == KeyEvent.Release then
        mouse_drag.drag_workspace = false
    end

    ui.dbgText(4, tostring(zooming.cx))
    ui.dbgText(5, tostring(zooming.cy))
    ui.dbgText(6, tostring(zooming.zoom))
end

return ui
