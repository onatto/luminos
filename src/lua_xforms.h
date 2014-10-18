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
int execPort(const char* port_name, char* error_msg, int env = 0);
int shutdownLua(int env = 0);

int port_programStart(const char* port_name, char* std_out, int env = 0);
