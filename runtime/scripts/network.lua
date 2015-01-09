local network = {}
local ffi = require 'ffi'
local debugger = require 'debugger'
local helpers = require 'helpers'
local ui = require 'ui'
local core = require 'core'

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
  local id = tonumber(args[1])
  local x = tonumber(args[2])
  local y = tonumber(args[3])
  local w = tonumber(args[4])
  local h = tonumber(args[5])
  local xform =      args[6]
  local name =       args[7]

  cmp = helpers.split(xform, '/')
  local module, submodule = cmp[1], cmp[2]
  ui.createNode(id,x,y,w,h,module,submodule)
end
local function CreateConnCmd(args)
  local id = tonumber(args[1])
  table.remove(args, 1)
  for i = 1, #args, 3 do
    local input_name = args[i]
    local out_node = tonumber(args[i+1])
    local output_name = args[i+2]
    debugger.print( "Creating connection for " .. input_name .. " " .. tostring(out_node) .. " " .. output_name)
    core.nodes[id].connections[input_name] = {out_node_id = out_node, port_name = output_name}
  end
end

local CmdMap = {
  createnode = CreateNodeCmd,
  connections = CreateConnCmd,
}

function portReceiveMessage(msg)
  debugger.print("Received msg: >>> " .. msg)
  args = SplitWhitespace(msg)
  cmd = args[1]
  table.remove(args, 1)
  CmdMap[cmd](args)
end
