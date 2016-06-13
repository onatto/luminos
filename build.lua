#!/usr/bin/lua
local os = require 'os'

function exec(str)
    local res = os.execute(str)
    print(res)
    if res > 0 then
        os.exit(-1)
    end
end

if arg[1] == 'vs' then
    exec("./tools/windows/genie.exe vs2012")
    exec("./tools/windows/genie.exe vs2013")
    os.exit(0)
end

if arg[1] == 'clang' then
    exec("./tools/linux/genie --gcc=linux-clang gmake")
    os.exit(0)
end

if arg[1] == 'clean' then
    exec("rm -rf .build/")
    os.exit(0)
end

if arg[1] == 'deb' then
	exec("cd runtime ; gdb ./luminosDebug")
    os.exit(0)
end

local prgname
local config
local builddir

if not arg[1] then
    arg[1] = 'd64'
end

if arg[1] == 'd32' then
    prgname = 'luminosDebug'
    config = 'debug32'
    builddir = 'linux32_clang'
elseif arg[1] == 'd64' then
    prgname = 'luminosDebug'
    config = 'debug64'
    builddir = 'linux64_clang'
elseif arg[1] == 'r32' then
    prgname = 'luminosRelease'
    config = 'release32'
    builddir = 'linux32_clang'
elseif arg[1] == 'r64' then
    prgname = 'luminosRelease'
    config = 'release64'
    builddir = 'linux64_clang'
end

exec ("make config=" .. config .. " -C .build/projects/gmake-linux-clang")
exec ("cp .build/" .. builddir .. "/bin/" .. prgname .. " runtime")
exec ("cd runtime ; ./" .. prgname .. " &")
