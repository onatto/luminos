#include "lua.hpp"
#include "lauxlib.h"

struct LuaEnvironments
{
    enum Enum
    {
        Default = 0,
        Count
    };
};

int initLua(int env = 0);
int compileLua(const char* filename, char* error_msg, int env = 0);
int shutdownLua(int env = 0);