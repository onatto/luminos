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
  C.coreStoreWorkspace()
end

return core
