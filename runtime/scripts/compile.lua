local test = [[ 
in f32 rad = 'is 6.0'
in f32 frequency
in f32 amplitude

out f32 sin

xform sin
out.sin = math.sin(in.rad * in.frequency) * in.amplitude
]]

local types = 
{
"f32",
"i8",
"i16",
"i32",
"i64",
"u8",
"u16",
"u32",
"u64",
"str" 
}

function parse_def(def)
end

function words_list(str)
    words = {}
    for word in str:gmatch("%S+") do 
        table.insert(words, word)
    end
    return words
end

function parse_transform(def)
    local xform = {}
    xform.inputs = {}
    xform.outputs = {}
    start_func = false
    for line in def:gmatch("[^\r\n]+") do
        words = words_list(line)
        if words[1] == 'in' then
            local input = { vtype = words[2], vname = words[3] }
            if words[4] then
                local s, e = string.find(line, '=')
                input.default = string.sub(line, e+1)
            end
            table.insert(xform.inputs, input)
        elseif words[1] == 'out' then
            local output = { vtype = words[2], vname = words[3] }
            table.insert(xform.outputs, output)
        elseif words[1] == 'xform' then
            xform.name = words[2]
            break
        end
    end

    local s, e = string.find(def, 'xform[%s+]' .. xform.name)
    xform.func_str = string.sub(def, e+1)
    return xform
end

function parse_func(def)
end

local test_xform = parse_transform(test, inputs, outputs)

print(test_xform.name)
print(test_xform.outputs[1].vname)
print(test_xform.inputs[1].default)
print(test_xform.func_str)
