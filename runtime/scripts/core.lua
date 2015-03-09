local core = {}
local debugger = require 'debugger'
local helpers = require 'helpers'
local lexer = require 'lexer'
local ffi = require 'ffi'

local C = ffi.C

helpers.cdef(ffi, "core.h")

core.nodes = {}
function core.execNode(node)
  if node.visited or node.cached then
    return node
  end

  local xform = node.xform
  -- This is the data binding stage for the xform
  local inputs = xform.inputs
  local connections = node.connections
  for input_idx, input_def in pairs(inputs) do -- Iterate over each input
      local input_name = xform.input_map[input_idx]
      local connection = rawget(connections, input_name)
      if connection then
      -- The input is non-constant
          local result_node = core.execNode(core.nodes[connection.out_node_id])
          node.input_values[input_name] = result_node.output_values[connection.port_name]
      else
      -- The input is a constant, each node has its unique constants
        if node.constants[input_name] then
          node.input_values[input_name] = node.constants[input_name]
        else
          node.input_values[input_name] = node.xform.inputs[input_idx].default
        end
      end
  end

  -- All inputs are ready at this point, evaluate the xform which sets up any outputs it can too

  lexer.xformFunc[xform.module][xform.name].eval(node.input_values, node.output_values)
  node.visited = true
  -- The outputs of this xform are ready, maybe they're there, maybe not
  return node
end

function core.programStart()
    for _k,node in pairs(core.nodes) do
      if node then
        node.visited = false
      end
    end
end


function core.saveWorkspace()
  C.coreNewWorkspace()
  for _, node in pairs(core.nodes) do
    if node then
      -- Store node
      C.coreStoreNode(node.id, node.x, node.y, node.w, node.h, node.xform.module, node.xform.name)
      -- Store connection
      for inputName, connection in pairs(node.connections) do
        C.coreStoreConnection(node.id, connection.out_node_id, inputName, connection.port_name)
      end
      -- Store consts
      for inputName, constant in pairs(node.constants) do
        local type = lexer.generaliseType(node.xform.inputs[node.xform.input_name_map[inputName]].type)
        if type == lexer.Types.Float then
          C.coreStoreConstNumber(node.id, inputName, constant)
        elseif type == lexer.Types.String then
          C.coreStoreConstStr(node.id, inputName, constant, string.len(constant))
        end
      end
    end
  end
  C.coreSaveWorkspace()
end

function core.cacheNode(node)
  for inputName, conn in pairs(node.connections) do
    core.cacheNode(core.nodes[conn.out_node_id])
  end
  core.execNode(node)
  local cacheFunc = lexer.xformFunc[node.xform.module][node.xform.name].cache
  if cacheFunc then
    cacheFunc(node.input_values, node.output_values)
  end
end

function core.createNode(id, x, y, w, h, module, submodule)
    local node = {}
    node.sx = x
    node.sy = y
    node.w = w
    node.h = h
    node.id = id
    node.hoverState = 0
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
    return node
end

function core.createConn(nodeInp, nodeOut, inpName, outName)
  core.nodes[nodeInp].connections[inpName] = {out_node_id = nodeOut, port_name = outName}
end

function core.defConst(node, input_name, const)
  core.nodes[node].constants[input_name] = const
end

function core.updateConn(nodeInp, nodeOut, inputName, outputName)
  local inpNode = core.nodes[nodeInp]
  local outNode = core.nodes[nodeOut]
  inpNode.connections[inputName] = {out_node_id = nodeOut, port_name = outputName}
  C.nw_send("UpdateConn " .. tostring(nodeInp) .. " " .. inputName .. " " .. tostring(nodeOut) .. " " .. outputName)
end

function core.deleteConn(nodeInp, inputName)
  core.nodes[nodeInp].connections[inputName] = nil
  C.nw_send("DeleteConn " .. tostring(nodeInp) .. " " .. inputName)
end

function core.sendNodePosUpdate(nodeID, x, y)
  local node = core.nodes[nodeID]
  C.nw_send("UpdateNodePos " .. tostring(node.id) .. " " .. tostring(node.sx) .. " " .. tostring(node.sy))
end

function core.deleteNode(nodeID)
  if C.nw_send("DeleteNode " .. tostring(nodeID)) == -1 then
    local node = core.nodes[nodeID]
    if node then
      local deleteFunc = lexer.xformFunc[node.xform.module][node.xform.name].delete
      if deleteFunc then
        deleteFunc(node.input_values,node.output_values)
      end
    end
    core.nodes[nodeID] = nil
  end
end

function core.updateConst(nodeID, inputName, constant)
  local node = core.nodes[nodeID]
  node.constants[inputName] = constant
  C.nw_send("UpdateConst " .. tostring(nodeID) .. " " .. inputName .. " " .. tostring(constant))
end

function core.deleteConst(nodeID, inputName)
  local node = core.nodes[nodeID]
  node.constants[inputName] = nil
  C.nw_send("DeleteConst " .. tostring(node.id) .. " " .. inputName)
end

function core.createNodeRequest(x, y, module, name)
  local xformTable = lexer.xform(module, name)
  local req = "CreateNode " .. table.concat({x, y, 180, 90,  module .. "/" .. name, xformTable.name}, " ")
  if C.nw_send(req) == -1 then
    core.createNode(#core.nodes + 1, x, y, 180, 90, module, name)
  end
end

coreCreateNode = core.createNode
coreCreateConn = core.createConn
coreDefConst = core.defConst

return core
