local core = {}
local debugger = require 'debugger'
local helpers = require 'helpers'
local lexer = require 'lexer'

core.nodes = {}
function core.execNode(node)
  -- xform.visited means that the output table for that xform was calculated(cached) from an earlier xform
  -- that is, that this vertex has been visited in the DAG traversal before

  local xform = node.xform
  if xform.visited or xform.cached then
    return node
  end
  -- This is the data binding stage for the xform
  local inputs = xform.inputs
  local connections = node.connections
  for input_idx, input_def in pairs(inputs) do -- Iterate over each input
      local input_name = xform.input_map[input_idx]
      local connection = rawget(connections, input_name)
      if connection then
      -- The input is non-constant
          local result_node = core.execNode(core.nodes[connection.out_node_id])
          xform.input_values[input_name] = result_node.xform.output_values[connection.port_name]
      else
      -- The input is a constant, each node has its unique constants
        xform.input_values[input_name] = node.constants[input_name]
      end
  end

  -- All inputs are ready at this point, evaluate the xform which sets up any outputs it can too
  lexer.xformFunc[xform.module][xform.name](xform.input_values, xform.output_values)
  xform.visited = true
  -- The outputs of this xform are ready, maybe they're there, maybe not
  return node
end

function core.cloneTransform(node, xform)
    local clone = helpers.deepCopy(xform)
    for input_idx,input in pairs(clone.inputs) do
      local input_name = xform.input_map[input_idx]
      node.constants[input_name] = input.default
    end
    return clone
end

function core.programStart()
    for _k,node in pairs(core.nodes) do
      if node then
        node.xform.visited = false
      end
    end
end

return core
