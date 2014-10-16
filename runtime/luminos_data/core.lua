function execTransform(transform)
    for input in transform.inputs do -- Iterate over each input
        -- If input is connected to another output (there exists a connection table)
        if rawget(input.connection) ~= nil then
            if #(input.connection.transform.output) > 0 then
                result = execTransform(input.connection.transform, transform)
                input.value = result.outputs[input.connection.name]
            end
        end
    end

    transform.outputs = transform.eval()
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
    inputs = { 
        str_a = {type = "string"},
        str_b = {type = "string"}
    },

    outputs = {
        concat_str = {}
    },

    eval = function()
         return { concat_str = str_a.value .. str_b.value }
    end
}

string_generator = {
       name = "String",
       str = "",

       eval = function()
               return str
       end
}
        
