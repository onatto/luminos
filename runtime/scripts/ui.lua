-- UI Module

local ui = {}
local ffi = require "ffi"

ffi.cdef
[[
    int ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
    void ui_dbgTextPrintf(int y, const char *str);
    uint8_t ui_getKeyboardState(uint16_t key);
]]

ui.drawNode = ffi.C.ui_drawNode
ui.dbgText = ffi.C.ui_dbgTextPrintf
ui.getKeyboardState = ffi.C.ui_getKeyboardState
local ui_nodes = {}

local BNDWidgetState = { Default = 0, Hover = 1, Active = 2 }

function ui.createNode(x, y, xform)
    local node = {}
    node.x = x
    node.y = y
    node.w = 180
    node.h = 40
    node.bndWidgetState = BNDWidgetState.Default
    node.xform = xform
    table.insert(ui_nodes, node)
    return node
end

local function drawNode(node_data)
    ui.drawNode(node_data.x, node_data.y, node_data.w, node_data.h, node_data.bndWidgetState, node_data.xform.name, 255, 50, 100, 255)
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
