local network = {}
local ffi = require 'ffi'
local debugger = require 'debugger'
local helpers = require 'helpers'

ffi.cdef
[[
    void nw_send(const char* msg);
]]

local function SplitWhitespace(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

function portReceiveMessageError(error_msg)
end

local function CreateNodeCmd(args)
  local x = tonumber(args[1])
  local y = tonumber(args[2])
  local w = tonumber(args[3])
  local h = tonumber(args[4])
  local xform =      args[5]
  local name =       args[6]
end
function portReceiveMessage(msg)
  args = SplitWhitespace(msg)
  cmd = args[1]
  table.remove(args, 1)
  debugger.print(msg)
end
