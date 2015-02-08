local debugger = {}

function debugger.init()
    local file = io.open('stacktrace.txt', 'w+')
    file:close()
    file = io.open('stdout.txt', 'w+')
    file:close()
    file = io.open('transform_tables.txt', 'w+')
    file:close()
    file = io.open('functions.txt', 'w+')
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

function debugger.print(str, path)
   local file
    if not path then
       file = io.open('stdout.txt', 'a+')
    else
       file = io.open(path, 'a+')
    end
    file:write(tostring(str) .. "\n")
    file:close()
end

-- Do not give depth and file parameters
function debugger.printTable(table, file, depth)
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
            debugger.printTable(v, file, depth+1)
        else
            str = str .. " := " .. tostring(v) .. "\n"
            file:write(str)
        end
    end
    if depth == 0 and file then
        file:close()
    end
end

return debugger
