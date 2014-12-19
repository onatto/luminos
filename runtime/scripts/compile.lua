local test = [[ 
module math_xforms
in f32 rad = 'is 6.0'
in f32 frequency
in f32 amplitude

out f32 sin

xform sin
out.sin = math.sin(inp.rad * inp.frequency) * inp.amplitude
]]

local function get_tokens(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

local function parse_transform_definition(def)
    local xform = {}
    xform.inputs = {}
    xform.outputs = {}
    start_func = false
    for line in def:gmatch("[^\r\n]+") do
        tokens = get_tokens(line)
        if tokens[1] == 'in' then
            local input = { vtype = tokens[2], vname = tokens[3] }
            if tokens[4] then
                local s, e = string.find(line, '[%s+]=[%s+]')
                input.default = string.sub(line, e+1)
            end
            table.insert(xform.inputs, input)
        elseif tokens[1] == 'out' then
            local output = { vtype = tokens[2], vname = tokens[3] }
            table.insert(xform.outputs, output)
        elseif tokens[1] == 'xform' then
            xform.name = tokens[2]
            break
        elseif tokens[1] == 'module' then
            xform.module = tokens[2]
        end
    end

    local s, e = string.find(def, 'xform[%s+]' .. xform.name)
    xform.func_str = string.sub(def, e+1)
    return xform
end

local function transform_table_str(xform)
    local def = ""
    def = def .. string.format("local %s = {}\n", xform.module)
    def = def .. string.format("%s.%s_transform = {\n", xform.module, xform.name)
    def = def .. string.format("name = '%s',\n", xform.name)
    def = def ..               "inputs = {\n"
    for _, input in pairs(xform.inputs) do
        def = def .. string.format("%s = {type = '%s', default=%s},\n", input.vname, input.vtype, input.default)
    end
    def = def .. "},\noutputs = {\n"
    for _, output in pairs(xform.outputs) do
        def = def .. string.format("%s = {type = '%s'},\n", output.vname, output.vtype)
    end
    def = def .. "},\n"
    def = def ..  [[
    inp = {},
    out = {},
    eval = function()
    ]]
    def = def .. xform.func_str
    def = def .. "\nend\n}\n"
    def = def .. "return " .. xform.module
    return def
end

local test_xform = parse_transform_definition(test, inputs, outputs)

-- print(test_xform.name)
-- print(test_xform.outputs[1].vname)
-- print(test_xform.inputs[1].default)
-- print(test_xform.func_str)

def = transform_table_str(test_xform)
--print(def)

func, err= loadstring(def)

if not func then
    print(err)
else
    table_def = func()
end
