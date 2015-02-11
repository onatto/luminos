enum FileLoadStatus
{
    FILELOAD_SUCCESS    = 0,
    FILELOAD_FAILOPEN   = -1,
    FILELOAD_FAILREAD   = -2,
};

int load_file(const char *filename, char** data, size_t* size_out);
void* debugOutputCallback(unsigned int src, unsigned int type, unsigned int id, 
        unsigned int severity, size_t len, const char* msg, const void* userParam );

#define LOG(msg) printf(msg);
#define LOGSTR(str) printf("%s\n", str)
#define LOGSTRL(str,len) fwrite(str, sizeof(char), len, stdout)
