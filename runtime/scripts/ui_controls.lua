local ffi = require("ffi")
ffi.cdef
[[
    int ui_drawNode(float x, float y, float w, float h, int widget_state, const char* title, char r, char g, char b, char a);
    void ui_dbgTextPrintf(int y, const char *str);
    int ui_getKeyboardState(uint16_t key);
]]

local drawNode = ffi.C.ui_drawNode
dbgText = ffi.C.ui_dbgTextPrintf
getKeyboardState = ffi.C.ui_getKeyboardState
local ui_nodes = {}

function create_node(x, y, xform)
    local node = {}
    node.x = x
    node.y = y
    node.w = 120
    node.h = 40
    node.xform = xform
    table.insert(ui_nodes, node)
    return node
end

function draw_nodes()
    for _k, node in pairs(ui_nodes) do
        draw_node(node)
    end
end

function draw_node(node_data)
    drawNode(node_data.x, node_data.y, node_data.w, node_data.h, 0, node_data.xform.name, 255, 50, 100, 255)
end

function pt_aabb_test(minx, miny, w, h, px, py)
    if minx < px and minx + w > px and miny < py and miny + h > py then
        return true
    end
    return false
end

function nodes_pt_intersect(point)
    for _k, node in pairs(ui_nodes) do
        insideAABB = pt_aabb_test(node.x, node.y, node.w, node.h, point.x, point.y)
        if insideAABB then
            return node
        end
    end
end

selected_nodes = {}
function select_ui_nodes()
    local mouse_pos = { x = g_mouseState.mx, y = g_mouseState.my }
    local node = nodes_aabb_test(mouse_pos)
    if node == nil then
        return
    end

    table.insert(selected_nodes, node)
    return node
end
