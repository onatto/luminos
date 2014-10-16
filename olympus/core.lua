function execTransform(transform)
    for input in transform.inputs -- Iterate over each input
        -- If input is connected to another output (there exists a connection table)
        if rawget(input.connection) is not nil 
            result = execTransform(input.connection.transform, transform)
            result.outputs[input.connection.name] = input.value
        end
    end

    transform.outputs = transform.eval()
    return transform
end


concat_transform = {
    name = "Concat",
    inputs = { 
        str1 = {type = "string"}
        str2 = {type = "string"}
    },

    outputs = {
        concat_str = {}
    },

    eval = function()
         return { concat_str = str1.value .. str2.value }
    end
}

string_generator = {
       name = "String" 
       str = ""

       eval = function()
               return str
       end
}
        
