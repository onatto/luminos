local debugger = {}

function debugger.dumpStack()
    local file = io.open('stacktrace.txt', 'a+')
    local traceback_str = debug.traceback()
    local dbg_info = debug.getinfo(2, "n").name
    file:write("\n@@@@@  " .. dbg_info .. " @@@@@\n")
    file:write(traceback_str)
    file:close()
end

return debugger
