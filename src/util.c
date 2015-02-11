#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "types.h"
#include "util.h"

#include "gl44.h"

int load_file(const char *filename, char** data, size_t* size_out)
{
    size_t size = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        *data = NULL;
        return FILELOAD_FAILOPEN; // -1 means file opening fail 
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *data = (char *)malloc(size + 1);
    if (size != fread(*data, sizeof(char), size, f))
    {
        free(*data);
        return FILELOAD_FAILREAD; // -2 means file reading fail 
    }
    fclose(f);
    (*data)[size] = 0;
    *size_out = size;
    return FILELOAD_SUCCESS;
}

void* debugOutputCallback(unsigned int src, unsigned int type, unsigned int id, 
        unsigned int severity, size_t len, const char* msg, const void* userParam )
{
  UNUSED(id); UNUSED(userParam);
  char* srcStr, *typeStr, *sevStr;
  switch(src)
  {
    case GL_DEBUG_SOURCE_API:
      srcStr = "GL";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      srcStr = "GLSL Compiler";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      srcStr = "Window System - GLX/WGL";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      srcStr = "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      srcStr = "Application";
      break;
    default:
      srcStr = "Other";
      break;
  }
  switch(type)
  {
    case GL_DEBUG_TYPE_ERROR:
      typeStr = "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      typeStr = "Deprecated";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      typeStr = "Undefined behavior";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      typeStr = "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      typeStr = "Performance";
      break;
    case GL_DEBUG_TYPE_OTHER:
      typeStr = "Other";
      break;
    case GL_DEBUG_TYPE_MARKER:
      typeStr = "Other";
      break;
    default:
      typeStr = "";
      break;
  }
  switch(severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:
      sevStr = "High";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      sevStr = "Medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      sevStr = "Low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      sevStr = "Notification";
      break;
  }

  printf("Debug Output:\n \
	  Source: %s\n \
	  Type: %s\n \
	  Severity: %s\n \
	  %.*s\n", srcStr, typeStr, sevStr, (int)len, msg);
}

