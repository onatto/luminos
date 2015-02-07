#pragma once

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT
#endif

struct lua_State;

//extern "C"
//{
    FFI_EXPORT void nw_send(const char* msg);
//}

void network_init(struct lua_State* l, const char* url, unsigned port);
void network_update();
void network_close();
void network_setlua(struct lua_State* l);
void network_flushw();
