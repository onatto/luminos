local core = {}
local debugger = require "debugger"
local helpers = require "helpers"
function core.execNode(node)
  -- transform.visited means that the output table for that transform was calculated(cached) from an earlier transform
  -- that is, that this vertex has been visited in the DAG traversal before

  local transform = node.xform
  if transform.visited or transform.cached then
    return transform
  end
  -- This is the data binding stage for the transform
  local inputs = transform.inputs
  local connections = node.connections
  for input_name, input_def in pairs(inputs) do -- Iterate over each input
      local connection = rawget(connections, input_name)
      if connection then
      -- The input is non-constant
          local result = core.execNode(connection.node)
          input_def.value = result.outputs[connection.port.name].value
      else
      -- The input is a constant, each node has its unique constants
        input_def.value = node.constants[input_name]
      end
  end

  -- All inputs are ready at this point, evaluate the transform which sets up any outputs it can too
  transform.eval(transform)
  transform.visited = true
  -- The outputs of this transform are ready, maybe they're there, maybe not
  return node
end

function core.cloneTransform(node, transform)
    local clone = helpers.deepCopy(transform)
    for input_name,input in pairs(clone.inputs) do
        node.constants[input_name] = input.default
    end
    return clone
end

core.concat_xform = {
    name = "Concat",
    -- Just change the default for that node instead of going one node deeper for constant inputs
    inputs = {
        str_a = {type = "string", default = ""},
        str_b = {type = "string", default = ""}
    },
    connections = {},
    outputs = {
        concat_str = {type = "string"}
    },
    eval = function(self)
         self.outputs.concat_str.value = self.inputs.str_a.value .. self.inputs.str_b.value
    end
}

core.mouse_xform = {
    name = "Mouse",
    inputs = {},
    connections = {},
    outputs = {
        mx = {type = "number"},
        my = {type = "number"}
    },
    eval = function(self)
        self.outputs.mx.value = g_mouseState.mx
        self.outputs.my.value = g_mouseState.my
    end
}

core.time_xform = {
    name = "Time",
    inputs = {},
    connections = {},
    outputs = {
        time = {type = "number"}
    },
    eval = function(self)
        self.outputs.time.value = g_time;
    end
}

return core
