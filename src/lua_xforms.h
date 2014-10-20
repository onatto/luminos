struct lua_State;
struct LuaEnvironments
{
    enum Enum
    {
        Default = 0,
        Count
    };
};
extern lua_State* states[LuaEnvironments::Count];

int initLua(int env = 0);
int compileLua(const char* filename, char* error_msg, int env = 0);
int execPort(const char* port_name, char* error_msg, int env = 0);
int shutdownLua(int env = 0);

int port_programStart(const char* port_name, char* std_out, int env = 0);
inline lua_State* get_luastate(int env) { return states[env]; }

int initEnvironmentVariables(int env = 0);
int uploadEnvironmentVariables(float time, int env = 0);

struct RecompileInput
{
    char* filename;
    char* status_msg;
    char* error_msg;
};
void cmdRecompile(const void* userdata);
