/*
 * Copyright 2011-2014 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.h"
#include <bgfx.h>

#include "SDL2/SDL.h"
#undef main
#include "SDL2/SDL_syswm.h"

#include "lua.hpp"
#include "lauxlib.h"

static char status_msg[256] = {0};
static char error_msg[2048] = {0};

bool quit = false;

int compileLua(lua_State* L, const char* filename)
{
	/* Load the file containing the script we are going to run */
    int status = luaL_loadfile(L, filename);
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
		strcpy(status_msg, "Couldn't load file:");
        strcpy(error_msg, lua_tostring(L, -1));
    }
	return status;
}

int init_sdl(SDL_Window** window, HWND* hwnd, const char* window_title, int width, int height)
{
	SDL_SysWMinfo info;
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		return 1;
	}
	//Setup our window and renderer
	*window = SDL_CreateWindow(window_title, 100, 100, width, height, SDL_WINDOW_SHOWN);
	if (*window == nullptr){
		SDL_Quit();
		return 2;
	}
	SDL_VERSION(&info.version); // initialize info structure with SDL version info
	if (SDL_GetWindowWMInfo(*window, &info)) {
		*hwnd = info.info.win.window;
	}
	else {
		// call failed
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	//uint32_t reset = BGFX_RESET_VSYNC;

	bgfx::init();
	bgfx::reset(width, height);

	lua_State *L;
    L = luaL_newstate();

	strcpy(status_msg, "Compiled Lua successfully!");
	strcpy(error_msg, "Description: Initialization and debug text.");
	compileLua(L, "luminos_data/core.lua");

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x303030ff
		, 1.0f
		, 0
		);

	SDL_Event e;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT){
				quit = true;
			}
			//Use number input to select which clip should be drawn
			if (e.type == SDL_KEYDOWN){
				switch (e.key.keysym.sym){
				case SDLK_ESCAPE:
					quit = true;
				break;
				case SDLK_F5:
					compileLua(L, "luminos_data/core.lua");
				break;
				}
			
			}
		}

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, width, height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::submit(0);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, status_msg);
		bgfx::dbgTextPrintf(0, 2, 0x6f, error_msg);

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
	}

/*
	while (!entry::processEvents(width, height, debug, reset) )
	{
		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, width, height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::submit(0);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, status_msg);
		bgfx::dbgTextPrintf(0, 2, 0x6f, error_msg);

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
	}
*/

	// Shutdown bgfx.
	bgfx::shutdown();

	lua_close(L);
	return 0;
}
