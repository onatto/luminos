local cmd = {}
local ffi = require("ffi")
ffi.cdef
[[
    void cmd_compile(const char* filename, char* status_msg_out, char* error_msg_out);
]]

cmd.compile = ffi.C.cmd_compile

return cmd
