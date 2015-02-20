local gfx = {}
local ffi = require 'ffi'
local helpers = require 'helpers'

helpers.cdef(ffi, "gfx.h")
helpers.cdef(ffi, "math.h")

return gfx
