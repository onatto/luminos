dofile "luminos/core.lua"
dofile "luminos/table_ops.lua"

concat_xform = deepcopy(concat_transform)
concat_xform.inputs.str_a.default = "Say "
concat_xform.inputs.str_b.default = "Hey"

concat_xform_2 = deepcopy(concat_transform)
concat_xform_2.connections.str_a = { transform = concat_xform, name = "concat_str" }
concat_xform_2.inputs.str_b.default = " Onat"

top_transform = {
    name = "stdout",
    inputs = {
        input_str = {type = "string", default = ""},
    },
    connections = {
      input_str = { transform = concat_xform_2, name = "concat_str" }
    },
    outputs = {
        stdout = {type = "string"}
    },
    eval = function(self)
         self.outputs.stdout.value = self.inputs.input_str.value
         print(self.outputs.stdout.value)
    end
}

execTransform(top_transform)