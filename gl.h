#pragma once
#include <Windows.h>
void *GetAnyGLFuncAddress(const char *name);
void initGL();
#define GLFUNC(returnType, convention, name, ...)         \
	typedef returnType (convention*name##GLFN)(__VA_ARGS__); \
	extern name##GLFN name;

GLFUNC(void, __stdcall, AWS_glBindTexture, unsigned int target, unsigned int texture);
GLFUNC(void, __stdcall, AWS_glGetTexLevelParameteriv, unsigned int target, int level, unsigned int pname, int *params);
GLFUNC(void, __stdcall, AWS_glTexImage2D, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels);
GLFUNC(unsigned int, __stdcall, AWS_glGetError, void);
GLFUNC(void, __stdcall, AWS_glTexParameteri, unsigned int, unsigned int, int);
GLFUNC(void, __stdcall, AWS_glEnable, unsigned int);
GLFUNC(void, __stdcall, AWS_glTexSubImage2D, unsigned int, int, int, int, int, int, unsigned int, unsigned int, void*);
GLFUNC(const char*, __stdcall, AWS_glGetString, unsigned int);
GLFUNC(void, __stdcall, AWS_glGenerateMipmap, unsigned int);
extern char* glVersion;
extern unsigned int glMajor;