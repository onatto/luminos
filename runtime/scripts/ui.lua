-- UI Module

local ui = {}
local ffi = require "ffi"
local debugger = require "debugger"
local helpers = require "helpers"
local core = require "core"

ffi.cdef
[[
    void ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
	void ui_drawPort(float x, float y, int widget_state, char r, char g, char b, char a);
	void ui_drawWire(float px, float py, float qx, float qy, int start_state, int end_state);
    void ui_dbgTextPrintf(int y, const char *str);
    uint8_t ui_getKeyboardState(uint16_t key);
    void ui_warpMouseInWindow(int x, int y);
]]

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
function ui.createNode(x, y, xform, constant_inputs)
    local node = {}
    node.x = x
    node.y = y
    node.w = 180
    node.h = 40
    node.bndWidgetState = BNDWidgetState.Default
    node.constants = {}
    node.connections = {}
    node.ports = {}
    node.xform = core.cloneTransform(node, xform)
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
        ui.drawWire(node_in.x + port_in.x * node_in.w,
                    node_in.y + port_in.y * node_in.h,
                    node_out.x + port_out.x * node_out.w,
                    node_out.y + port_out.y * node_out.h,
                    BNDWidgetState.Active, BNDWidgetState.Active)
    end
end

function ui.drawNodes()
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
    anchorx = nil,  -- Starting(Anchor) point of the node before dragging, new_pos = anchor + delta
    anchory = nil,
}
local hovered_node = nil
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
        if port_from then
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
            mouse_drag.anchorx = node_from.x + node_from.w * port_from.x
            mouse_drag.anchory = node_from.y + node_from.h * port_from.y
            mouse_drag.drag_connector = true
        end
    end

    -- If dragging, draw the wire when holding LMB
    if g_mouseState.left == KeyEvent.Hold and mouse_drag.drag_connector then
        ui.drawWire(mouse_drag.anchorx, mouse_drag.anchory, mx, my, BNDWidgetState.Active, BNDWidgetState.Active)
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
        end
        mouse_drag.drag_connector = false
        node_from = nil
        port_from = nil
        node_to = nil
        port_to = nil
    end
end

function ui.dragNodes()
    local mx, my = g_mouseState.mx, g_mouseState.my

    -- If not dragging a node already, find a node to drag
    if not mouse_drag.drag_node then
        hovered_node = nodes_pt_intersect(mx, my)
    end

    -- On LMB Press and not intersecting any nodes, start dragging
    if g_mouseState.left == KeyEvent.Press then
        if hovered_node and not (port_from or port_to) then
            mouse_drag.mx = mx
            mouse_drag.my = my
            mouse_drag.anchorx = hovered_node.x
            mouse_drag.anchory = hovered_node.y
            mouse_drag.drag_node = true
        end
    end

    -- Dragging a node if holding left mouse and we're dragging nodes
    if (g_mouseState.left == KeyEvent.Hold and mouse_drag.drag_node) then
        hovered_node.x = mouse_drag.anchorx + mx - mouse_drag.mx
        hovered_node.y = mouse_drag.anchory + my - mouse_drag.my
        hovered_node.bndWidgetState = BNDWidgetState.Active
    end

    -- On LMB release, stop dragging nodes
    if g_mouseState.left == KeyEvent.Release then
        if mouse_drag.drag_node then
            mouse_drag.drag_node = false
        end
    end
end


local selected_nodes = {}
local function select_ui_nodes()
    local mouse_pos = { x = g_mouseState.mx, y = g_mouseState.my }
    local node = nodes_aabb_test(mouse_pos)
    if node == nil then
        return
    end

    table.insert(selected_nodes, node)
    return node
end
return ui
