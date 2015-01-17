-- May the programming gods forgive me for the sins committed in this unholy lexer
-- I'll get to fixing this soon
--
local lexer = {}
local debugger = require 'debugger'

local function SplitWhitespace(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

local function ParseTransform(def)
    local xform = {}
    xform.inputs = {}
    xform.outputs = {}
    start_func = false
    for line in def:gmatch("[^\r\n]+") do
        tokens = SplitWhitespace(line)
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
        elseif tokens[1] == 'module' then
            xform.module = tokens[2]
        elseif tokens[1] == 'viz' then
            xform.viz = tokens[2]
        elseif tokens[1] == 'dispname' then
            xform.dispname = string.sub(line, 9)
        elseif tokens[1] == 'xform' then
            xform.name = tokens[2]
            break
        end
    end

    local s, e = string.find(def, 'xform[%s+]' .. xform.name)
    xform.func_str = string.sub(def, e+1)

    if not xform.dispname then
        xform.dispname = xform.name
    end
    return xform
end

lexer.xformFunc = {}
lexer.xformTable = {}
local function CreateTransformTable(xform)
    local def = ""
    local xformDef = ""
    -- FFI
    -- Transform module and name
    def = def .. string.format("%s_xforms.%s_transform = {\n", xform.module, xform.name)
    -- Transform name
    def = def .. string.format("name = '%s',\n", xform.name)
    -- Default display name of the transform, can be overwritten
    if xform.dispname then
        def = def .. string.format("dispname = '%s',\n", xform.dispname)
    end
    -- Visualization for this xform 
    if xform.viz then
        def = def .. string.format("viz = '%s',\n", xform.viz)
    end
    -- Inputs
    def = def .. "input_values = {},\n output_values = {},\n"
    def = def ..               "inputs = {\n"
    for _, input in pairs(xform.inputs) do
        def = def .. string.format("%s = {type = '%s', default=%s},\n", input.vname, input.vtype, tostring(input.default))
    end
    -- Outputs
    def = def .. "},\noutputs = {\n"
    for _, output in pairs(xform.outputs) do
        def = def .. string.format("%s = {type = '%s'},\n", output.vname, output.vtype)
    end
    def = def .. "}}\nreturn "
    def = def .. string.format("%s_xforms.%s_transform", xform.module, xform.name)
    -- Eval Function
    
    xformDef = "local ffi = require 'ffi'\n"
    xformDef = xformDef .. "local C = ffi.C\n"
    xformDef = xformDef .. "return function(inp, out)\n" 
    xformDef = xformDef .. xform.func_str .. "\nend"
    return def, xformDef
end

local function ReadFile(path)
    local f = io.open(path, "r")
    if not f then
        return nil
    end
    local content = f:read("*all")
    f:close()
    return content
end

lexer.lex = function(module, submodule)
    local path = "xforms/" .. module .. "/" .. submodule .. ".lua"
    local def = ReadFile(path)
    if not def then
        return nil
    end
    local parsed_xform  = ParseTransform(def)
    local tableCode, funcCode = CreateTransformTable(parsed_xform)
    debugger.print(tableCode, "transform_tables.txt")
    debugger.print(funcCode, "functions.txt")

    local tableFunc, err= loadstring(tableCode)

    if not tableFunc then
        debugger.print(err)
    else
        local transform = tableFunc()
        transform.module = module
        transform.submodule = submodule
        if not lexer.xformTable[module] then
            lexer.xformTable[module] = {}
        end
        lexer.xformTable[module][submodule] = transform
    end

    local xformFunc, err = loadstring(funcCode)
    if not xformFunc then
        debugger.print(err)
    else
        if not lexer.xformFunc[module] then
            lexer.xformFunc[module] = {}
        end
        lexer.xformFunc[module][submodule] = xformFunc()
    end

    if xformTable and xformFunc then
        return xformTable
    else
        return nil
    end
end

lexer.getTransform = function(module, submodule)
    if not lexer.xformFunc[module] then
        return nil
    end
    if not lexer.xformFunc[module][submodule] then
        local xformTable = lexer.lex(module,submodule)
        return xformTable
    else
        return lexer.xformTable[module][submodule]
    end
end

return lexer
