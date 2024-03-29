local network = {}
local ffi = require 'ffi'
local debugger = require 'debugger'
local helpers = require 'helpers'
local ui = require 'ui'
local core = require 'core'
local lexer = require 'lexer'

ffi.cdef
[[
    int nw_send(const char* msg);
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
  core.createNode(id,x,y,w,h,module,submodule)
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

local function DeleteConnCmd(args)
  local id = tonumber(args[1]) -- Node ID
  local input_name = args[2]
  core.nodes[id].connections[input_name] = nil
end

local function DeleteNodeCmd(args)
  local id = tonumber(args[1])
  local node = core.nodes[id]
  local deleteFunc = lexer.xformFunc[node.xform.module][node.xform.name].delete
  
  if deleteFunc then
    deleteFunc(node.input_values,node.output_values)
  end
  core.nodes[id] = nil

  -- It doesn't stop here, must delete all connections to this node as well!
  -- Iterate over all inputs of all nodes and delete connections to this node
end

local function ReadWord(str, start)
  s, e = string.find(str, "%S+", start)
  if s then
    return string.sub(str,s,e), e+1
  end
end

local function DefNodeConstant(node, input_name, const)
  local type
  if not node then
    return
  end
  for idx, input in ipairs(node.xform.inputs) do
    if input.name == input_name then
      type = input.type
      break
    end
  end
  node.constants[input_name] = lexer.convertFromString(const, type)
end

local function ConstCmd(cmd)
  local head, command, id, input_name, len, const
  head = 1
  command, head = ReadWord(cmd, head) -- const
  id, head = ReadWord(cmd, head) -- id
  id = tonumber(id)
  debugger.print("Got const command " .. cmd .. " for id " .. tostring(id))

  while head do
    debugger.print("Defining new const")
    input_name, head = ReadWord(cmd, head)
    if head then
      len, head = ReadWord(cmd, head)
      len = tonumber(len)
      const = string.sub(cmd, head+1, head+len)
      debugger.print("Const for: " .. input_name .. " -- " .. const .. " with head " .. tostring(head))
      DefNodeConstant(core.nodes[id], input_name, const)
      head = head+len+1
    end
  end
end

local CmdMap = {
  createnode = CreateNodeCmd,
  connections = CreateConnCmd,
  deletenode = DeleteNodeCmd,
  deleteconn = DeleteConnCmd,
}

function portReceiveMessage(msg)
  debugger.print("Received msg: >>> " .. msg)
  cmds = helpers.split(msg, string.char(4))
  for _, command_str in pairs(cmds) do
    debugger.print("Received cmd: >>> " .. command_str)
    args = SplitWhitespace(command_str)
    cmd = args[1]
    debugger.print("cmd is: " .. cmd)
    table.remove(args, 1)
    if CmdMap[cmd] then
      CmdMap[cmd](args)
    end
    if cmd == "consts" then
      ConstCmd(string.sub(command_str, 1, #command_str-1))
    end
  end
end
