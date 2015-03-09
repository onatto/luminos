#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "types.h"
#include "core.h"
#include "string.h"
#include "util.h"

#include "stdlib.h"

struct lua_State* s_luaState = NULL;
char s_statusMsg[256] = {0};
char s_errorMsg[2048] = {0};
const char* s_errorPort = NULL;

struct lua_State* getLuaState(){ return s_luaState; }
char* getStatusMsg() { return s_statusMsg; }
char* getErrorMsg() { return s_errorMsg; }

/* Start running the program @path = program_lua */
void coreInit(const char* program_lua, char* error_msg)
{
    if (s_luaState ) {
        lua_close(s_luaState);
    }
    s_luaState = luaL_newstate();
    lua_State* L = getLuaState();

    luaL_openlibs(L);
    /* Load the file containing the script we are going to run */
    int result = luaL_loadfile(L, program_lua);
    if (result) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        memset(error_msg, 0, strlen(error_msg));
        strcpy(error_msg, lua_tostring(L, -1));
    }
    else {
        result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (result) {
            memset(error_msg, 0, strlen(error_msg));
            strcpy(error_msg, lua_tostring(L, -1));
        }
    }

    if (result) {
        /* Something is wrong with the script */
        lua_pushstring(L, "Couldn't load file:");
        lua_setglobal(L, "g_statusMsg");
        lua_pushstring(L, error_msg);
        lua_setglobal(L, "g_errorMsg");
        return;
    }

    lua_pushnumber(L, 0.0);
    lua_setglobal(L, "g_time");
    lua_pushstring(L, "Luminos");
    lua_setglobal(L, "g_statusMsg");
}

void coreShutdown()
{
    lua_State* L = getLuaState();
    if (L != NULL) {
        lua_close(L);
    }
    s_luaState = NULL;
}

int coreExecPort(const char* port_name)
{
    lua_State* L = getLuaState();
    // Top index before the port function call - so the function can return multiple variables
    int top = lua_gettop(L);
    lua_getglobal(L, "portDisplayRuntimeError");
    lua_getglobal(L, port_name);
    if (!lua_isfunction(L, -1))
        return -1;

    /* Ask Lua to run the global function with name 'port_name' */
    /* portDisplayRuntimeError is the error handler, see lua_pcall doc for details */
    int result = lua_pcall(L, 0, LUA_MULTRET, -2);
    if (result) {
        s_errorPort = port_name;
        return -result;
    }
    else {
        s_errorPort = NULL;
    }

    int nresults = lua_gettop(L) - top;
    return nresults;
}


struct Node
{
    uint32 id;
    int32 x;
    int32 y;
    uint16 w;
    uint16 h;
    uint32 moduleOffset;
    uint32 xformOffset;
};
struct Connection
{
    uint32 inputNodeID;
    uint32 outputNodeID;
    uint32 inputNameOffset;
    uint32 outputNameOffset;
};
struct Constant
{
    uint32 id;
    uint32 inputNameOffset;
    union {
        double val;
        uint32 strOffset;
    };
};
typedef struct Node Node;
typedef struct Connection Connection;
typedef struct Constant Constant;
#define STRING_POOL_SIZE 1024*1024
#define MAX_NODES 1024

typedef struct Buffer {
    uint8* data;
    uint32 offset;
} Buffer;

static void initBuffer(Buffer* b, size_t size) {
    b->data = malloc(size);
    b->offset = 0;
    memset(b->data, 0, size);
}
static void deleteBuffer(Buffer* b) {
    free(b->data);
    memset(b, 0, sizeof(Buffer));
}
static void writeBuffer(Buffer* b, const void* data, uint32 size) {
    memcpy(b->data + b->offset, data, size);
    b->offset += size;
}
static void writeBufferFile(Buffer* b, FILE* f)
{
    fwrite(&b->offset, 4, 1, f);
    fwrite(b->data, b->offset, 1, f);
}
static void readBufferFile(Buffer* b, FILE* f)
{
    uint32 bytesToRead = 0;
    fread(&bytesToRead, 4, 1, f);
    fread(b->data, bytesToRead, 1, f);
    b->offset = bytesToRead;
}

static Buffer s_strPool;
static Buffer s_nodePool;
static Buffer s_constNumberPool;
static Buffer s_constStrPool;
static Buffer s_connectionPool;

static uint16 nodeCnt = 0;
static uint16 connCnt = 0;
static uint16 constNumberCnt = 0;
static uint16 constStrCnt = 0;
void coreDumpData()
{
    int ii;
    for (ii = 0; ii < nodeCnt; ii++) {
        Node* n = (Node*)(s_nodePool.data + ii*sizeof(Node));
        uint16 len_mod = (uint16)s_strPool.data[n->moduleOffset];
        uint16 len_xform = (uint16)s_strPool.data[n->xformOffset];
        uint8* mod = s_strPool.data + n->moduleOffset+2;
        uint8* xform = s_strPool.data + n->xformOffset+2;
        printf("Node %d, with x:%d y:%d w:%d h:%d ", n->id, n->x, n->y, n->w, n->h);
        printf(" module: %.*s xform: %.*s\n", len_mod, mod, len_xform, xform);
    }
    for (ii = 0; ii < connCnt; ii++) {
        Connection* c = (Connection*)(s_connectionPool.data + ii*sizeof(Connection));
        uint16 len_in = (uint16)s_strPool.data[c->inputNameOffset];
        uint16 len_out = (uint16)s_strPool.data[c->outputNameOffset];
        uint8* inp = s_strPool.data + c->inputNameOffset + 2;
        uint8* out = s_strPool.data + c->outputNameOffset + 2;
        printf("Connection %d -> %d  %.*s -> %.*s\n", 
                c->inputNodeID, 
                c->outputNodeID,
                len_in,
                inp,
                len_out,
                out);
    }
    for (ii = 0; ii < constStrCnt; ii++) {
        Constant* c = (Constant*)(s_constStrPool.data + ii * sizeof(Constant));
        uint16 len_inp = s_strPool.data[c->inputNameOffset];
        uint16 len_constant = s_strPool.data[c->strOffset];
        uint8* inp = s_strPool.data + c->inputNameOffset + 2;
        uint8* constant = s_strPool.data + c->strOffset + 2;
        printf("Const for id:%d -- %.*s: %.*s\n", c->id, len_inp, inp, len_constant, constant);
    }
    for (ii = 0; ii < constNumberCnt; ii++) {
        Constant* c = (Constant*)(s_constNumberPool.data + ii * sizeof(Constant));
        uint16 len_inp = s_strPool.data[c->inputNameOffset];
        uint8* inp = s_strPool.data + c->inputNameOffset + 2;
        printf("Const for id:%d -- %.*s: %f\n", c->id, len_inp, inp, c->val);
    }
}

void coreSetupWorkspace()
{
   int ii;
   lua_State* L = getLuaState(); 

   //function core.createNode(id, x, y, w, h, module, submodule)
   lua_getglobal(L, "coreCreateNode");
   for (ii = 0; ii < nodeCnt; ii++) {
       Node* n = (Node*)(s_nodePool.data + ii*sizeof(Node));
       uint16 len_mod = (uint16)s_strPool.data[n->moduleOffset];
       uint16 len_xform = (uint16)s_strPool.data[n->xformOffset];
       uint8* mod = s_strPool.data + n->moduleOffset+2;
       uint8* xform = s_strPool.data + n->xformOffset+2;
       
       lua_pushvalue(L, -1);
       lua_pushinteger(L, n->id);
       lua_pushinteger(L, n->x);
       lua_pushinteger(L, n->y);
       lua_pushinteger(L, n->w);
       lua_pushinteger(L, n->h);
       lua_pushlstring(L, mod, len_mod);
       lua_pushlstring(L, xform, len_xform);
       lua_call(L, 7, 0);
   }
   lua_pop(L, 1);
   // function core.createConn(nodeInp, nodeOut, inpName, outName)
   lua_getglobal(L, "coreCreateConn");
   for (ii = 0; ii < connCnt; ii++) {
       Connection* c = (Connection*)(s_connectionPool.data + ii*sizeof(Connection));
       uint16 len_in = (uint16)s_strPool.data[c->inputNameOffset];
       uint16 len_out = (uint16)s_strPool.data[c->outputNameOffset];
       uint8* inp = s_strPool.data + c->inputNameOffset + 2;
       uint8* out = s_strPool.data + c->outputNameOffset + 2;

       lua_pushvalue(L, -1);
       lua_pushinteger(L, c->inputNodeID);
       lua_pushinteger(L, c->outputNodeID);
       lua_pushlstring(L, inp, len_in);
       lua_pushlstring(L, out, len_out);
       lua_call(L, 4, 0);
   }
   lua_pop(L, 1);
   //function core.defConst(node, input_name, const)
   lua_getglobal(L, "coreDefConst");
   for (ii = 0; ii < constStrCnt; ii++) {
       Constant* c = (Constant*)(s_constStrPool.data + ii * sizeof(Constant));
       uint16 len_inp = s_strPool.data[c->inputNameOffset];
       uint16 len_constant = s_strPool.data[c->strOffset];
       uint8* inp = s_strPool.data + c->inputNameOffset + 2;
       uint8* constant = s_strPool.data + c->strOffset + 2;

       lua_pushvalue(L, -1);
       lua_pushinteger(L, c->id);
       lua_pushlstring(L, inp, len_inp);
       lua_pushlstring(L, constant, len_constant);
       lua_call(L, 3, 0);
   }
   for (ii = 0; ii < constNumberCnt; ii++) {
       Constant* c = (Constant*)(s_constNumberPool.data + ii * sizeof(Constant));
       uint16 len_inp = s_strPool.data[c->inputNameOffset];
       uint8* inp = s_strPool.data + c->inputNameOffset + 2;

       lua_pushvalue(L, -1);
       lua_pushinteger(L, c->id);
       lua_pushlstring(L, inp, len_inp);
       lua_pushnumber(L, c->val);
       lua_call(L, 3, 0);
   }
   lua_pop(L, 1);
}

void coreNewWorkspace()
{
    initBuffer(&s_strPool, STRING_POOL_SIZE);
    initBuffer(&s_nodePool, MAX_NODES       * sizeof(Node));
    initBuffer(&s_constStrPool, MAX_NODES * 10 * sizeof(Constant));
    initBuffer(&s_constNumberPool, MAX_NODES * 10 * sizeof(Constant));
    initBuffer(&s_connectionPool, MAX_NODES * 10 * sizeof(Connection));
    nodeCnt = 0;
    connCnt = 0;
    constNumberCnt = 0;
    constStrCnt = 0;
}

void coreDumpWorkspace()
{
    coreDumpData();
    deleteBuffer(&s_strPool);
    deleteBuffer(&s_nodePool);
    deleteBuffer(&s_constNumberPool);
    deleteBuffer(&s_constStrPool);
    deleteBuffer(&s_connectionPool);
}

void coreSaveWorkspace()
{
    FILE* f = fopen("workspace.data", "wb");
    if (f == NULL)
        return;
    fwrite(&nodeCnt, sizeof(uint16_t), 1, f);
    fwrite(&connCnt, sizeof(uint16_t), 1, f);
    fwrite(&constNumberCnt, sizeof(uint16_t), 1, f);
    fwrite(&constStrCnt, sizeof(uint16_t), 1, f);
    writeBufferFile(&s_strPool, f);
    writeBufferFile(&s_nodePool, f);
    writeBufferFile(&s_constNumberPool, f);
    writeBufferFile(&s_constStrPool, f);
    writeBufferFile(&s_connectionPool, f);
    fclose(f);
}

void coreLoadWorkspace()
{
    FILE* f = fopen("workspace.data", "rb");
    if (f == NULL)
        return;
    fread(&nodeCnt, sizeof(uint16_t), 1, f);
    fread(&connCnt, sizeof(uint16_t), 1, f);
    fread(&constNumberCnt, sizeof(uint16_t), 1, f);
    fread(&constStrCnt, sizeof(uint16_t), 1, f);
    readBufferFile(&s_strPool, f);
    readBufferFile(&s_nodePool, f);
    readBufferFile(&s_constNumberPool, f);
    readBufferFile(&s_constStrPool, f);
    readBufferFile(&s_connectionPool, f);
    fclose(f);
}

void coreStoreNode(uint32_t id, int32_t x, int32_t y, uint16_t w, uint16_t h, const char* module, const char* xform)
{
    Node n;
    n.id = id;
    n.x = x;
    n.y = y;
    n.w = h;
    n.h = w;
    uint16 len_mod = (uint16)strlen(module);
    uint16 len_xform = (uint16)strlen(xform);
    n.moduleOffset = s_strPool.offset;
    writeBuffer(&s_strPool, &len_mod, 2);
    writeBuffer(&s_strPool, module, len_mod);
    n.xformOffset = s_strPool.offset;
    writeBuffer(&s_strPool, &len_xform, 2);
    writeBuffer(&s_strPool, xform, len_xform);
    writeBuffer(&s_nodePool, &n, sizeof(Node));
    nodeCnt++;
}

void coreStoreConstNumber(uint32_t id, const char* inputName, double constant)
{
    Constant c;
    c.id = id;
    c.val = constant;
    c.inputNameOffset = s_strPool.offset;
    uint16 len_inputName = (uint16)strlen(inputName);
    writeBuffer(&s_strPool, &len_inputName, 2);
    writeBuffer(&s_strPool, inputName, len_inputName);
    writeBuffer(&s_constNumberPool, &c, sizeof(Constant));
    constNumberCnt++;
}

void coreStoreConstStr(uint32_t id, const char* inputName, const char* constant, uint16_t size)
{
    Constant c;
    c.id = id;
    c.strOffset = s_strPool.offset;
    writeBuffer(&s_strPool, &size, 2);
    writeBuffer(&s_strPool, constant, size);
    c.inputNameOffset = s_strPool.offset;
    uint16 len_inputName = (uint16)strlen(inputName);
    writeBuffer(&s_strPool, &len_inputName, 2);
    writeBuffer(&s_strPool, inputName, len_inputName);
    writeBuffer(&s_constStrPool, &c, sizeof(Constant));
    constStrCnt++;
}

void coreStoreConnection(uint32_t inpID, uint32_t outID, const char* inpName, const char* outName)
{
    Connection c;
    c.inputNodeID = inpID;
    c.outputNodeID = outID;
    uint16 len_inpName = (uint16)strlen(inpName);
    uint16 len_outName = (uint16)strlen(outName);

    c.inputNameOffset = s_strPool.offset;
    writeBuffer(&s_strPool, &len_inpName, 2);
    writeBuffer(&s_strPool, inpName, len_inpName);
    c.outputNameOffset = s_strPool.offset;
    writeBuffer(&s_strPool, &len_outName, 2);
    writeBuffer(&s_strPool, outName, len_outName);
    writeBuffer(&s_connectionPool, &c, sizeof(Connection));
    connCnt++;
}
