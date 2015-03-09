#pragma once

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

struct lua_State;

//extern "C"
//{
    FFI_EXPORT int nw_send(const char* msg);
//}

void networkInit(struct lua_State* l, const char* url, unsigned port);
void networkUpdate();
void networkShutdown();
void networkSetLua(struct lua_State* l);
void networkFlushWrites();
