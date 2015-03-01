local gfx = {}
local ffi = require 'ffi'
local helpers = require 'helpers'
local lexer = require 'lexer'
local C = ffi.C

helpers.cdef(ffi, "gfx.h")
helpers.cdef(ffi, "math.h")

local maxUniforms = 256
local uniformNames     = ffi.new("uint8_t[" .. maxUniforms * 32 .. "]", {})
local uniformTypes     = ffi.new("uint8_t[" .. maxUniforms .. "]", {})
local uniformLocations = ffi.new("uint8_t[" .. maxUniforms .. "]", {})

local ShaderTypes = {}
ShaderTypes[C.SHADER_VERT] = "vs"
ShaderTypes[C.SHADER_FRAG] = "fs"
ShaderTypes[C.SHADER_COMP] = "cs"
ShaderTypes[C.SHADER_GEOM] = "gs"

local UniformTypes = {}
UniformTypes[C.UNIFORM_FLOAT] = "f32"
UniformTypes[C.UNIFORM_VEC2] = "vec2"
UniformTypes[C.UNIFORM_VEC3] = "vec2"
UniformTypes[C.UNIFORM_VEC4] = "vec2"
UniformTypes[C.UNIFORM_MAT2] = "mat2"
UniformTypes[C.UNIFORM_MAT3] = "mat3"
UniformTypes[C.UNIFORM_MAT4] = "mat4"
UniformTypes[C.UNIFORM_SAMPLER1D] = "tex1D"
UniformTypes[C.UNIFORM_SAMPLER2D] = "tex2D"
UniformTypes[C.UNIFORM_SAMPLER3D] = "tex3D"
UniformTypes[C.UNIFORM_SAMPLER_CUBE] = "texCube"
UniformTypes[C.UNIFORM_IMAGE1D] = "img1D"
UniformTypes[C.UNIFORM_IMAGE2D] = "img2D"
UniformTypes[C.UNIFORM_IMAGE2D_ARRAY] = "img2DArr"
UniformTypes[C.UNIFORM_IMAGE3D] = "img3D"
UniformTypes[C.UNIFORM_TYPES] = "undef"

local BindFunc = {}
BindFunc[C.UNIFORM_IMAGE2D] = function(img_unit, img)
    C.gfxBindImage2D(img.hnd, img_unit, img.format)
end

function gfx.shaderDef(name, path, program, shaderType, uniforms)
   local shaderDef = {}
   shaderDef.name = name
   shaderDef.inputs = {}
   shaderDef.outputs = {}
   shaderDef.input_map = {}
   shaderDef.output_map = {}
   shaderDef.input_name_map = {}
   shaderDef.output_name_map = {}
   shaderDef.funcs = {}
   shaderDef.cache = {}
   shaderDef.imports = {}

   shaderDef.program = program
   
   --print("Shader " .. path)
   -- Inputs for the shader - to be displayed
   local samplerCount = 0
   local imageCount   = 0
   for idx, uniform in ipairs(uniforms) do
      --print("Uniform " .. uniform.name .. " of type " .. UniformTypes[uniform.type] .. ", location: " .. uniform.location)
      local input = { type = UniformTypes[uniform.type], name = uniform.name, uniformType = uniform.type}
      if uniform.type > C.UNIFORM_MAT4  and uniform.type < C.UNIFORM_IMAGE1D then
        input.location = samplerCount
        --C.gfxSetImageUnit(program, uniform.location, samplerCount)
        samplerCount = samplerCount + 1
      elseif uniform.type > C.UNIFORM_SAMPLER_CUBE then
        input.location = imageCount
        --C.gfxSetImageUnit(program, uniform.location, imageCount)
        imageCount = imageCount + 1
      else
        input.location = uniform.location
      end

      table.insert(shaderDef.inputs, input)
      table.insert(shaderDef.input_map, input.name)
      shaderDef.input_name_map[input.name] = #shaderDef.inputs
   end

   -- Output for the shader
   shaderDef.outputs[1] = { type = shaderType, name = "hnd" }
   shaderDef.output_map[1] = "hnd"
   shaderDef.output_name_map.hnd = 1

   -- Funcs
   shaderDef.funcs.eval = function(inp, out)
     for idx, input in pairs(inp) do
       BindFunc[shaderDef.inputs[idx].uniformType](shaderDef.inputs[idx].location, input_value)
     end
   end
end

function gfx.loadShader(name, path, type)
  local program = C.gfxCreateShader(path, type)
  if program == 0 then
    return
  end
  local uniformCount = C.gfxGetShaderUniforms(program, uniformNames, maxUniforms * 32, uniformTypes, uniformLocations)
  tt = ffi.string(uniformNames)
  print(tt)
  local nameOffset = 0
  local uniforms = {}
  for i=0,uniformCount-1 do
    local uniform = {}
    uniform.name = ffi.string(uniformNames + nameOffset)
    uniform.type = uniformTypes[i]
    uniform.location = uniformLocations[i]
    table.insert(uniforms, uniform)
    nameOffset = nameOffset + #name + 1 -- +1 for the NULL terminator
  end
  gfx.shaderDef(name, path, program, ShaderTypes[type], uniforms)
end

gfx.loadShader("SSQUAD", "shaders/quad_tex.vert", C.SHADER_VERT)
gfx.loadShader("BLUR", "shaders/blur_h.cs", C.SHADER_COMP)
gfx.loadShader("SSQUAD FRAG", "shaders/ssquad.frag", C.SHADER_FRAG)

return gfx
