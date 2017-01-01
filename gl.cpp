#include <Windows.h>
#include "gl.h"
/*GLFUNC(void, __stdcall, glBindTexture, unsigned int target, unsigned int texture);
GLFUNC(void, __stdcall, glGetTeLevelParameteriv, unsigned int target, int level, unsigned int pname, int *params);
GLFUNC(void, __stdcall, glTexImage2D, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels);*/
AWS_glBindTextureGLFN AWS_glBindTexture;
AWS_glGetTexLevelParameterivGLFN AWS_glGetTexLevelParameteriv;
AWS_glTexImage2DGLFN AWS_glTexImage2D;
AWS_glGetErrorGLFN AWS_glGetError;
AWS_glTexParameteriGLFN AWS_glTexParameteri;
AWS_glEnableGLFN AWS_glEnable;
AWS_glTexSubImage2DGLFN AWS_glTexSubImage2D;
void *GetAnyGLFuncAddress(const char *name)
{
	void *p = (void *)wglGetProcAddress(name);
	if (p == 0 ||
		(p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
		(p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, name);
	}

	return p;
}
void initGL() {
	AWS_glBindTexture = (AWS_glBindTextureGLFN)GetAnyGLFuncAddress("glBindTexture");
	AWS_glGetTexLevelParameteriv = (AWS_glGetTexLevelParameterivGLFN)GetAnyGLFuncAddress("glGetTexLevelParameteriv");
	AWS_glTexImage2D = (AWS_glTexImage2DGLFN)GetAnyGLFuncAddress("glTexImage2D");
	AWS_glGetError = (AWS_glGetErrorGLFN)GetAnyGLFuncAddress("glGetError");
	AWS_glTexParameteri = (AWS_glTexParameteriGLFN)GetAnyGLFuncAddress("glTexParameteri");
	AWS_glEnable = (AWS_glEnableGLFN)GetAnyGLFuncAddress("glEnable");
	AWS_glTexSubImage2D = (AWS_glTexSubImage2DGLFN)GetAnyGLFuncAddress("glTexSubImage2D");
}