#!/usr/bin/lua
local os = require 'os'

function exec(str)
    local res = os.execute(str)
    print(res)
    if res > 0 then
        os.exit(-1)
    end
end

local prgname
local config
local builddir

if not arg[1] then
    arg[1] = 'D64'
end

if arg[1] == 'D32' then
    prgname = 'luminosDebug'
    config = 'debug32'
    builddir = 'linux32_clang'
elseif arg[1] == 'D64' then
    prgname = 'luminosDebug'
    config = 'debug64'
    builddir = 'linux64_clang'
elseif arg[1] == 'R32' then
    prgname = 'luminosRelease'
    config = 'release32'
    builddir = 'linux32_clang'
elseif arg[1] == 'R64' then
    prgname = 'luminosRelease'
    config = 'release64'
    builddir = 'linux64_clang'
end

exec ("cd /home/onat/code/luminos/")
exec ("make config=" .. config .. " -C .build/projects/gmake-linux-clang")
exec ("cp .build/" .. builddir .. "/bin/" .. prgname .. " runtime")
exec ("cd runtime ; ./" .. prgname .. " &")
