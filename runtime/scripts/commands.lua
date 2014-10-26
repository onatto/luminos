local ffi = require("ffi")
ffi.cdef
[[
    void cmd_compile(const char* filename, char* status_msg_out, char* error_msg_out);
]]

recompile = ffi.C.cmd_compile
