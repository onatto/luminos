#pragma once

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT
#endif

struct lua_State;

extern "C"
{
    FFI_EXPORT void nw_send(const char* msg);
}

void network_init(lua_State* l, const char* url);
void network_update();
void network_close();
void network_setlua(lua_State* l);
