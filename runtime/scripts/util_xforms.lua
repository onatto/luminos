local util = {}
local ui = require 'ui'
local core = require 'core'
local debugger = require 'debugger'
local ffi = require 'ffi'

local C = ffi.C

util.print_xform = {
    name = "Print Text",
    inputs = {
        str = {type = "string", default=""},
        x = {type = "number", default=0},
        y = {type = "number", default=600},
    },
    outputs = {
    },
    eval = function(self)
       C.ui_drawText(self.inputs.x.value, self.inputs.y.value, self.inputs.str.value)
    end
}

return util
