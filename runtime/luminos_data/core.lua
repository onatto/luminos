function execTransform(transform)
  -- transform.visited means that the output table for that transform was calculated(cached) from an earlier transform
  -- that is, that this vertex has been visited in the DAG traversal before
  
  if transform.visited then
    return transform
  end
  
  local inputs = transform.inputs
  local connections = transform.connections
  for input_name, input_def in pairs(inputs) do -- Iterate over each input
      local connection = rawget(connections, input_name)
      if connection then
          local result = execTransform(connection.transform)
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

concat_transform = {
    name = "Concat",
    
    -- Just change the default for that node instead of going one node deeper for constant inputs
    inputs = {
        str_a = {type = "string", default = ""},
        str_b = {type = "string", default = ""}
    },
    
    connections = {
    },

    outputs = {
        concat_str = {type = "string"}
    },

    eval = function(self)
         self.outputs.concat_str.value = self.inputs.str_a.value .. self.inputs.str_b.value
    end
}

-- We don't need this complexity anymore, change default value in the inputs table for constant inputs
  string_generator = {
       name = "String",
       inputs = {},
       str = "",

       eval = function()
          return str
       end
}