local lexer = {}
local helpers  = require 'helpers'
local debugger = require 'debugger'

local function SplitWhitespace(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

local Types = { Float = 0, Integer = 1, String = 2, VecN = 3, Table = 4}
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
        return Types.Table
    end
end

lexer.generaliseType = GeneraliseType
lexer.Types = Types

lexer.convertFromString = function(Str, Type)
    local type = GeneraliseType(Type)
    if type == Types.Float then
        return tonumber(Str)
    elseif type == Types.String then
        return tostring(Str)
    end
end

lexer.convertToString = function(Val, Type)
    if not Val then
        return nil
    end
    local type = GeneraliseType(Type)
    if type == Types.Float then
        return string.format("%.2f", Val)
    else
        return tostring(Val)
    end
end

local Keywords = {}
local currentDef
local currentFunc
local definitions = {}
local ParsingFunc

local function ParseInput(tokens, line)
    local input = { type = tokens[1], name = tokens[2] }
    if input.type == "str" then
       input.default = ""
    end
    if tokens[4] then
       local s, e = string.find(line, '[%s+]=[%s+]')
       local default = string.sub(line, e+1)
       input.default = lexer.convertFromString(default, tokens[1])
    end
    table.insert(currentDef.inputs, input)
    table.insert(currentDef.input_map, input.name)
    currentDef.input_name_map[input.name] = #currentDef.inputs
end
local function ParseOutput(tokens, line)
    local output = { type = tokens[1], name = tokens[2] }
    table.insert(currentDef.outputs, output)
    table.insert(currentDef.output_map, output.name)
    currentDef.output_name_map[output.name] = #currentDef.outputs
end

Keywords.xform = function(tokens)
   currentDef = {}
   currentDef.name = tokens[2]
   currentDef.inputs = {}
   currentDef.outputs = {}
   -- Map inputs/outputs idx to input/output name
   currentDef.input_map = {}
   currentDef.output_map = {}
   currentDef.input_name_map = {}
   currentDef.output_name_map = {}
   currentDef.funcs = {}
   currentDef.cache = {}
   currentDef.imports = {}

   definitions[currentDef.name] = currentDef
   ParsingFunc = nil
   end
Keywords.module = function(tokens)
   currentDef.module = tokens[2]
   ParsingFunc = nil
   end
Keywords.inputs = function(tokens)
   ParsingFunc = ParseInput 
   end
Keywords.outputs = function(tokens)
    ParsingFunc = ParseOutput
   end
Keywords.func = function(tokens, head)
   funcName = tokens[2]
   currentDef.funcs[funcName] = {}
   currentFunc = currentDef.funcs[funcName]
   currentFunc.starts = head
   ParsingFunc =nil
end
Keywords.import = function(tokens)
   local import = tokens[2]
   local importAs
   if tokens[3] == "as" and tokens[4] then
      importAs = tokens[4]
   else
      importAs = import
   end
   currentDef.imports[importAs] = import
end

local function ParseTransform(def)
   definitions = {}
   head = 1
   for line in def:gmatch("[^\r\n]+") do
      tokens = SplitWhitespace(line)
      if tokens[1] == "func" then
         if currentFunc then
            head = string.find(def, line, head)
            currentFunc.ends = head - 1
            currentFunc = nil
         end
         funcName = tokens[2]
         currentDef.funcs[funcName] = {}
         currentFunc = currentDef.funcs[funcName]
         head = string.find(def, line, head)
         currentFunc.starts = head + string.len(line)
         ParsingFunc =nil
      elseif tokens[1] == "dispname" then
         currentDef.dispname = string.sub(line, 9)
      elseif Keywords[tokens[1]] then
         if currentFunc then
            head = string.find(def, line, head)
            currentFunc.ends = head - 1
            currentFunc = nil
         end
         Keywords[tokens[1]](tokens, head)
      elseif ParsingFunc then
         ParsingFunc(tokens, line)
      end
      head = head + string.len(line)
   end
   if currentFunc then
      currentFunc.ends = string.len(def) + 1
      currentFunc = nil
   end

   return definitions
end

lexer.xformFunc = {}
lexer.xformTable = {}


lexer.lex = function(module, submodule)
    local path = "xforms/" .. module .. "/" .. submodule .. ".lua"
    local def = helpers.readFile(path)
    if not def then
        return nil
    end

    local definitions  = ParseTransform(def)

    for name, xform in pairs(definitions) do
       if not lexer.xformTable[xform.module] then
          lexer.xformTable[xform.module] = {}
       end

       if not xform.dispname then
          xform.dispname = xform.name
       end
       lexer.xformTable[xform.module][xform.name] = xform

       for funcName, funcDef in pairs(xform.funcs) do
          local funcBody = string.sub(def, funcDef.starts, funcDef.ends)
          local func = "local ffi = require 'ffi'\n"
          func = func .. "local C = ffi.C\n"
          func = func .. "local SDL = require 'sdlkeys'\n"
          for importAs, import in pairs(xform.imports) do
             func = func .. "local " .. importAs .. " = require '" .. import .. "'\n"
          end
          func = func .. "return function(inp, out)\n" 
          func = func .. funcBody .. "\nend"

          local xformFunc, err = loadstring(func)
          if not xformFunc then
             debugger.print(err)
             debugger.print("----------------")
             debugger.print(func)
             return nil
          else
             if not lexer.xformFunc[xform.module] then
                lexer.xformFunc[xform.module] = {}
             end
             if not lexer.xformFunc[xform.module][xform.name] then
                lexer.xformFunc[xform.module][xform.name] = {}
             end
             lexer.xformFunc[xform.module][xform.name][funcName] = xformFunc()
          end
       end
    end
end

lexer.xform = function(module, xform_name)
    if not lexer.xformFunc[module] then
        return nil
    end
    if not lexer.xformFunc[module][xform_name] then
        lexer.lex(module,xform_name)
     end
     return lexer.xformTable[module][xform_name]
end

return lexer
