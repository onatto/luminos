void coreNewWorkspace();
void coreSaveWorkspace();
void coreLoadWorkspace();
void coreStoreNode(uint32_t id, int32_t x, int32_t y, uint16_t w, uint16_t h, const char* module, const char* xform);
void coreStoreConstNumber(uint32_t id, const char* inputName, double constant);
void coreStoreConstStr(uint32_t id, const char* inputName, const char* constant, uint16_t size);
void coreStoreConnection(uint32_t inpID, uint32_t outID, const char* inpName, const char* outName);
