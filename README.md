luminos
=======

luminos will be a non-OO visual programming environment, putting emphasis on understanding & visualizing data flow
Or, that was the idea when I started this project, it was experimental and I had no idea what I was doing.
Beware, the code causes major cringe(for me anyways) and bleeding eyes.

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

Oh, rename the SDL2 folder(3rdparty/SDL2) to something else if you are on Linux.

Running
-----------------
redis-server
go run scripts/server.go
lua build.lua

Usage
-----
Be aware, this is an usability hell.
To select the node creation menu, hold LMB and click RMB, click on the node(core/mouse for example).
Move the selected nodes around with holding LMB, select multiple nodes with CTRL+LMB, delete nodes with delete,
drag connections from the connection ports and cycle through the constant input list with TAB and change the constant input with BACKSPACE.
Your work will be stored in the redis database.

Screenshot
----------
Because who wants to go through the hell that I just described above anyway.

![Alt text](/screenshot.png?raw=true "Optional Title")
