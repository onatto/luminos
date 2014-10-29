-- UI Module

local ui = {}
local ffi = require "ffi"
local debugger = require "debugger"
local helpers = require "helpers"

ffi.cdef
[[
    void ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
	void ui_drawPort(float x, float y, int widget_state, char r, char g, char b, char a);
    void ui_dbgTextPrintf(int y, const char *str);
    uint8_t ui_getKeyboardState(uint16_t key);
]]

ui.drawNode = ffi.C.ui_drawNode
ui.drawPort = ffi.C.ui_drawPort
ui.dbgText = ffi.C.ui_dbgTextPrintf
ui.getKeyboardState = ffi.C.ui_getKeyboardState

local BNDWidgetState = { Default = 0, Hover = 1, Active = 2 }

local ui_nodes = {}
function ui.createNode(x, y, xform)
    local node = {}
    node.x = x
    node.y = y
    node.w = 180
    node.h = 40
    node.bndWidgetState = BNDWidgetState.Default
    node.xform = xform
    node.inports = {}

    -- Calculate port locations
    local i = 1
    local input_cnt = helpers.tableLength(node.xform.inputs)
    for input_name, input in pairs(node.xform.inputs) do
        local port = { name = input_name }
        port.x = (1/(input_cnt+1)) * i
        port.y = 0.85
        port.bndWidgetState = BNDWidgetState.Default
        table.insert(node.inports, port)
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
    for i, port in ipairs(node.inports) do
        ui.drawPort(node.x + port.x * node.w, node.y + port.y * node.h, port.bndWidgetState, 0, 100, 255, 255)
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
local mouse_drag =
{
    mx = nil,
    my = nil,
    anchorx = nil,
    anchory = nil,
    dragging = false
}
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
    local radius = 0.004
    for _k, port in pairs(node.inports) do
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

local selected_node = nil
local selected_port_from = nil
local selected_port_to = nil
function ui.dragNodes()
    local mx, my = g_mouseState.mx, g_mouseState.my
    debugger.mouseData(8)
    if (not mouse_drag.dragging) then
        selected_node = nodes_pt_intersect(mx, my)
        if selected_node then
            local relx, rely = pt_aabb_relative(selected_node.x, selected_node.y, selected_node.w, selected_node.h, mx, my)
            if not selected_port_from then
                selected_port_from = ports_pt_intersect(selected_node, relx, rely)
            else
                selected_port_to = ports_pt_intersect(selected_node, relx, rely)
            end
        end
    end

    if (g_mouseState.left == KeyEvent.Press) then
        if (selected_node) then
            mouse_drag.mx = mx
            mouse_drag.my = my
            mouse_drag.nodex = selected_node.x
            mouse_drag.nodey = selected_node.y
            mouse_drag.dragging = true
        end
    end

    if (g_mouseState.left == KeyEvent.Hold and mouse_drag.dragging) then
        selected_node.x = mouse_drag.nodex + mx - mouse_drag.mx
        selected_node.y = mouse_drag.nodey + my - mouse_drag.my
        selected_node.bndWidgetState = BNDWidgetState.Active
    end

    if (g_mouseState.left == KeyEvent.Release and mouse_drag.dragging) then
        mouse_drag.dragging = false
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
