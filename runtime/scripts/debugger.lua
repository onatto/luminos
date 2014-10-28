local debugger = {}

function debugger.init()
    local file = io.open('stacktrace.txt', 'w+')
    file:close()
    file = io.open('stdout.txt', 'w+')
    file:close()
end

function debugger.dumpStack()
    local file = io.open('stacktrace.txt', 'a+')
    local traceback_str = debug.traceback()
    local dbg_info = debug.getinfo(2, "n").name
    file:write("Function name: " .. dbg_info .. " \n")
    file:write(traceback_str)
    file:close()
end

function debugger.print(str)
    local file = io.open('stdout.txt', 'a+')
    file:write(tostring(str) .. "\n")
    file:close()
end

function debugger.printTable(table, depth, file)
    if not depth then
        depth = 0
    end
    if depth == 0 and not file then
        file = io.open('stdout.txt', 'a+')
    end
    for k,v in pairs(table) do
        local indent = ""
        for ind = 1, depth do
            indent = indent .. "  "
        end
        str = indent .. tostring(k)
        if type(v) == "table" then
            str = str .. ": \n"
            file:write(str)
            debugger.printTable(v, depth+1, file)
        else
            str = str .. " := " .. tostring(v) .. "\n"
            file:write(str)
        end
    end
    if depth == 0 then
        file:close()
    end
end

return debugger
