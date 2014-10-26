--
-- Copyright 2011-2014 Onat Turkcuoglu. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--
-- Edited from bgfx

LUMINOS_DIR = path.getabsolute("./../") .. "/"
BGFX_DIR = LUMINOS_DIR .. "../bgfx/"
BX_DIR = LUMINOS_DIR .. "../bx/"
LUA_DIR = LUMINOS_DIR .. "../luajit-2.0/"
-- SDL2_DIR = LUMINOS_DIR .. "

solution "luminos"
	configurations {
		"Debug",
		"Release",
	}

    language "C++"

	platforms {
		"x32",
		"x64",
--		"Xbox360",
		"Native", -- for targets where bitness is not specified
	}

local LUMINOS_BUILD_DIR = (LUMINOS_DIR .. ".build/")
local LUMINOS_THIRD_PARTY_DIR = (LUMINOS_DIR .. "3rdparty/")

dofile (BX_DIR .. "scripts/toolchain.lua")
toolchain(LUMINOS_BUILD_DIR, LUMINOS_THIRD_PARTY_DIR)

function luminosProject()

    _name = "luminos"
    project (_name)
    uuid (os.uuid(_name))
    kind "WindowedApp"

    configuration {}
    defines { "UNICODE" }

    debugdir (LUMINOS_DIR .. "runtime/")

    includedirs {
        LUMINOS_DIR .. "3rdparty",
        BX_DIR .. "include",
        BGFX_DIR .. "include",
        LUA_DIR .. "src",
    }

    files {
        LUMINOS_DIR .. "src/**.cpp",
        LUMINOS_DIR .. "src/**.c",
        LUMINOS_DIR .. "src/**.h",
        LUMINOS_DIR .. "3rdparty/nanovg/*.cpp"
    }

    links {
        "lua51",
        "SDL2"
    }

    libdirs {
        LUA_DIR .. "/src",
    }

	configuration { "x32", "windows*" }
        libdirs {
            BGFX_DIR .. ".build/win32_" .. _ACTION .. "/bin"
        }
	configuration { "x64", "windows*" }
        libdirs {
            BGFX_DIR .. ".build/win64_" .. _ACTION .. "/bin"
        }

    configuration { "Debug" }
        links {
            "bgfxDebug",
        }
    configuration { "Release" }
        links {
            "bgfxRelease",
        }

    configuration { "x32", "windows" }
        libdirs { "$(SDL2_DIR)/lib/x86" }

    configuration { "x64", "windows" }
        libdirs { "$(SDL2_DIR)/lib/x64" }

    configuration { "vs*" }
        linkoptions {
            "/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
        }
        links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
            "DelayImp",
        }

    configuration { "vs201*" }
        linkoptions { -- this is needed only for testing with GLES2/3 on Windows with VS201x
            "/DELAYLOAD:\"libEGL.dll\"",
            "/DELAYLOAD:\"libGLESv2.dll\"",
        }

    configuration { "windows" }
        links {
            "gdi32",
            "psapi",
        }

    configuration { "android*" }
        kind "ConsoleApp"
        targetextension ".so"
        linkoptions {
            "-shared",
        }
        links {
            "EGL",
            "GLESv2",
        }

    configuration { "nacl or nacl-arm" }
        kind "ConsoleApp"
        targetextension ".nexe"
        links {
            "ppapi",
            "ppapi_gles2",
            "pthread",
        }

    configuration { "pnacl" }
        kind "ConsoleApp"
        targetextension ".pexe"
        links {
            "ppapi",
            "ppapi_gles2",
            "pthread",
        }

    configuration { "asmjs" }
        kind "ConsoleApp"
        targetextension ".bc"

    configuration { "linux-*" }
        links {
            "X11",
            "GL",
            "pthread",
        }

    configuration { "rpi" }
        links {
            "X11",
            "GLESv2",
            "EGL",
            "bcm_host",
            "vcos",
            "vchiq_arm",
            "pthread",
        }

    configuration { "osx" }
        files {
            BGFX_DIR .. "examples/common/**.mm",
        }
        links {
            "Cocoa.framework",
            "OpenGL.framework",
        }

    configuration { "xcode4" }
        platforms {
            "Universal"
        }
        files {
            BGFX_DIR .. "examples/common/**.mm",
        }
        links {
            "Cocoa.framework",
            "Foundation.framework",
            "OpenGL.framework",
        }

    configuration { "ios*" }
        kind "ConsoleApp"
        files {
            BGFX_DIR .. "examples/common/**.mm",
        }
        linkoptions {
            "-framework CoreFoundation",
            "-framework Foundation",
            "-framework OpenGLES",
            "-framework UIKit",
            "-framework QuartzCore",
        }

    configuration { "qnx*" }
        targetextension ""
        links {
            "EGL",
            "GLESv2",
        }

    configuration {}

    strip()
end

luminosProject()
