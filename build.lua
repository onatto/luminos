local os = require 'os'

function exec(str)
    os.execute(str)
end

local prgname
local config
local builddir

if not arg[1] then
    arg[1] = 'D32'
end

if arg[1] == 'D32' then
    prgname = 'luminosDebug'
    config = 'debug32'
    builddir = 'linux32_gcc'
elseif arg[1] == 'D64' then
    prgname = 'luminosDebug'
    config = 'debug64'
    builddir = 'linux64_gcc'
elseif arg[1] == 'R32' then
    prgname = 'luminosRelease'
    config = 'release32'
    builddir = 'linux32_gcc'
elseif arg[1] == 'R64' then
    prgname = 'luminosRelease'
    config = 'release64'
    builddir = 'linux64_gcc'
end

exec ("make config=" .. config .. " -C .build/projects/gmake-linux")
exec ("cp .build/" .. builddir .. "/bin/" .. prgname .. " runtime")
exec ("cd runtime ; ./" .. prgname .. " &")
