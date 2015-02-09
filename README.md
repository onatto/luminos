luminos
=======

luminos will be a non-OO visual programming environment, putting emphasis on understanding & visualizing data flow

Dependencies
-------------
SDL2
luajit2.0

Installation
------------
Set $(SDL2_DIR) and $(LUAJIT_DIR) environment variables, otherwise they are expected at the parent folder of luminos

For Visual Studio project files:
lua build.lua vs

Building for Linux:
lua build.lua clang
lua build.lua d64
