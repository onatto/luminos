local cmd = {}
local ffi = require("ffi")
ffi.cdef
[[
    int cmd_restart(const char* filename, char* status_msg_out, char* error_msg_out);
]]

cmd.restart = ffi.C.cmd_restart

return cmd
