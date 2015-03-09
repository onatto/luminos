#pragma once

void coreInit(const char* program_lua, char* error_msg_out);
/* Execute Lua function <port_name> */
int coreExecPort(const char* port_name);
void coreShutdown();

#ifdef MSVC
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

FFI_EXPORT void coreNewWorkspace();
FFI_EXPORT void coreSaveWorkspace();
FFI_EXPORT void coreLoadWorkspace();
FFI_EXPORT void coreSetupWorkspace();
FFI_EXPORT void coreStoreNode(uint32_t id, int32_t x, int32_t y, uint16_t w, uint16_t h, const char* module, const char* xform);
FFI_EXPORT void coreStoreConstNumber(uint32_t id, const char* inputName, double constant);
FFI_EXPORT void coreStoreConstStr(uint32_t id, const char* inputName, const char* constant, uint16_t size);
FFI_EXPORT void coreStoreConnection(uint32_t inpID, uint32_t outID, const char* inpName, const char* outName);

/* States by the core module */
struct lua_State;

extern struct lua_State* s_luaState;
extern char s_statusMsg[256];
extern char s_errorMsg[2048];
extern const char* s_errorPort;

struct lua_State* getLuaState();
char* getStatusMsg();
char* getErrorMsg();
