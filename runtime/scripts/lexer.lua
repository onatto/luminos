local lexer = {}
local debugger = require 'debugger'

local function SplitWhitespace(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

local Types = { Float = 0, Integer = 1, String = 2, VecN = 3, Other = 4}
local GeneraliseType = function(Type)
    if Type == 'f16' or Type == 'f32' or Type == 'f64' then
        return Types.Float
    elseif Type == 'i8' or Type == 'i16' or Type == 'i32' or Type == 'i64' or  Type == 'u8' or Type == 'u16' or Type == 'u32' or Type == 'u64' then
        return Types.Float
    elseif Type == 'str' then
        return Types.String
    elseif Type == 'vec2' or Type == 'vec3' or Type == 'vec4' then
        return Types.VecN
    else
        return Types.Other
    end
end

local function ParseTransform(def)
    local xform = {}
    xform.inputs = {}
    xform.outputs = {}
    xform.input_values = {}
    xform.output_values = {}
    -- Map inputs/outputs idx to input/output name
    xform.input_map = {}
    xform.output_map = {}
    for line in def:gmatch("[^\r\n]+") do
        tokens = SplitWhitespace(line)
        if tokens[1] == 'in' then
            local input = { type = tokens[2], name = tokens[3] }
            if tokens[4] then
                local s, e = string.find(line, '[%s+]=[%s+]')
                local type = GeneraliseType(tokens[2])
                local default = string.sub(line, e+1)
                if type == Types.Float then
                    input.default = tonumber(default)
                elseif type == Types.String then
                    input.default = tostring(default)
                end
            end
            table.insert(xform.inputs, input)
            table.insert(xform.input_map, input.name)
        elseif tokens[1] == 'out' then
            local output = { type = tokens[2], name = tokens[3] }
            table.insert(xform.outputs, output)
            table.insert(xform.output_map, output.name)
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

    local xform  = ParseTransform(def)
    if not lexer.xformTable[xform.module] then
        lexer.xformTable[xform.module] = {}
    end
    lexer.xformTable[xform.module][xform.name] = xform

    func = "local ffi = require 'ffi'\n"
    func = func .. "local C = ffi.C\n"
    func = func .. "return function(inp, out)\n" 
    func = func .. xform.func_str .. "\nend"

    local xformFunc, err = loadstring(func)
    if not xformFunc then
        debugger.print(err)
        return nil
    else
        if not lexer.xformFunc[xform.module] then
            lexer.xformFunc[xform.module] = {}
        end
        lexer.xformFunc[xform.module][xform.name] = xformFunc()
        return xform
    end
end

lexer.xform = function(module, xform_name)
    if not lexer.xformFunc[module] then
        return nil
    end
    if not lexer.xformFunc[module][xform_name] then
        local xformTable = lexer.lex(module,xform_name)
        return xformTable
    else
        return lexer.xformTable[module][xform_name]
    end
end

return lexer
