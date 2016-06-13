luminos
=======

luminos will be a non-OO visual programming environment, putting emphasis on understanding & visualizing data flow

Dependencies
-------------
SDL2
luajit2.0
redis
golang

Installation
------------
Set $(SDL2_DIR) and $(LUAJIT_DIR) environment variables, otherwise they are expected at ../luajit/

Generate Visual Studio project files:
lua build.lua vs

Building for Linux:
lua build.lua clang
lua build.lua d64

Running
-----------------
redis-server
go run scripts/server.go
cd runtime
./luminosDebug

Screenshot
----------

![Alt text](/screenshot.png?raw=true "Optional Title")
