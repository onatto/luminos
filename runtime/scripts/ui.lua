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
    local marginx = 0.1 -- for port locations, in relative coordinates
    local marginw = 1-2*marginx -- for port locations, in relative coordinates
    local node = {}
    node.x = x
    node.y = y
    node.w = 180
    node.h = 40
    node.bndWidgetState = BNDWidgetState.Default
    node.xform = xform
    node.inports = {}

    -- Calculate port locations
    local i = 0
    local input_cnt = helpers.tableLength(node.xform.inputs)
    for input_name, input in pairs(node.xform.inputs) do
        local port = { name = input_name }
        port.x = marginx + (marginw / input_cnt) * i
        port.y = 0.9
        table.insert(node.inports, port)
    end
    table.insert(ui_nodes, node)
    return node
end

local function drawNode(node)
    ui.drawNode(node.x, node.y, node.w, node.h, node.bndWidgetState, node.xform.name, 255, 50, 100, 255)
    for i, port in ipairs(node.inports) do
        ui.drawPort(node.x + port.x * node.w, node.y + port.y * node.h, 1, 255, 50, 100, 255)
    end
end

function ui.drawNodes()
    for _k, node in pairs(ui_nodes) do
        drawNode(node)
    end
end

local mouse_drag =
{
    mx = nil,
    my = nil,
    anchorx = nil,
    anchory = nil,
    dragging = false
}
local selected_node = nil
local function pt_aabb_test(minx, miny, w, h, px, py)
    if minx < px and px < minx + w and miny < py and py < miny + h then
        return true
    end
    return false
end
local function pt_aabb_relative(minx, miny, w, h, px, py)
    return (px - minx) / w, (py - miny) / h
end

local function nodes_pt_intersect(px, py)
    local isect = nil
    for _k, node in pairs(ui_nodes) do
        insideAABB = pt_aabb_test(node.x, node.y, node.w, node.h, px, py)
        if insideAABB and not isect then
            local relx, rely = pt_aabb_relative(node.x, node.y, node.w, node.h, px, py)
            ui.dbgText(15, tostring(relx))
            ui.dbgText(16, tostring(rely))
            node.bndWidgetState = BNDWidgetState.Hover
            isect = node
        else
            node.bndWidgetState = BNDWidgetState.Default
        end
    end
    return isect
end
function ui.dragNodes()
    if (not mouse_drag.dragging) then
        selected_node = nodes_pt_intersect(g_mouseState.mx, g_mouseState.my)
    end
    if (g_mouseState.left == KeyEvent.Press) then
        if (selected_node) then
            mouse_drag.mx = g_mouseState.mx
            mouse_drag.my = g_mouseState.my
            mouse_drag.nodex = selected_node.x
            mouse_drag.nodey = selected_node.y
            mouse_drag.dragging = true
        end
    end

    if (g_mouseState.left == KeyEvent.Hold and mouse_drag.dragging) then
        selected_node.x = mouse_drag.nodex + g_mouseState.mx - mouse_drag.mx
        selected_node.y = mouse_drag.nodey + g_mouseState.my - mouse_drag.my
        selected_node.bndWidgetState = BNDWidgetState.Active
    end

    if (g_mouseState.left == KeyEvent.Release and mouse_drag.dragging) then
        mouse_drag.dragging = false
    end
end


selected_nodes = {}
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
