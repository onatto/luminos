#include "network.h"
#include "ws_client.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lua.hpp"
#include "lauxlib.h"

static wsconn* ws;
static lua_State* L;

void receive_data(void* data, size_t size)
{
    lua_getglobal(L, "portReceiveMessageError");
    lua_getglobal(L, "portReceiveMessage");
    if (!lua_isfunction(L, -1))
        return;
    lua_pushlstring(L, (const char*)data, size);
    lua_pcall(L, 1, 0, -2);
}

void network_init(lua_State* l, const char* url)
{
    L = l;
    ws = open_wsconn(url, 1, "");
}

void nw_send(const char* msg)
{
    sendText(ws, msg);
}

void network_update()
{
    poll(ws, 0);
    dispatch(ws, receive_data);
}

void network_close()
{
    closeconn(ws);
    wsconn_shutdown(ws);
}

void network_setlua(lua_State* l)
{
    L = l;
}

