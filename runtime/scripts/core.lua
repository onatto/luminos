local core = {}
local debugger = require "debugger"
function core.execTransform(transform)
  -- transform.visited means that the output table for that transform was calculated(cached) from an earlier transform
  -- that is, that this vertex has been visited in the DAG traversal before

  if transform.visited or transform.cached then
    return transform
  end

  local inputs = transform.inputs
  local connections = transform.connections
  for input_name, input_def in pairs(inputs) do -- Iterate over each input
      local connection = rawget(connections, input_name)
      if connection then
          local result = core.execTransform(connection.transform)
          input_def.value = result.outputs[connection.name].value
      else
        input_def.value = input_def.default
      end
  end

  -- All inputs are ready at this point, evaluate the transform which sets up any outputs it can too
  transform.eval(transform)
  transform.visited = true
  -- The outputs of this transform are ready, maybe they're there, maybe not
  return transform
end

-- Here str_a and str_b are input names
-- Nodes should be evaluated only once
-- That means, the calculated outputs must be copied in the transform itself
-- to their connected nodes, outputs feed the inputs
-- Outputs have connection infos too
--

core.concat_transform = {
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
