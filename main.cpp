#pragma once
#include <Windows.h>
#include "torque.h"
#include <gl/GL.h>
#include <Awesomium\WebCore.h>
#include <Awesomium\STLHelpers.h>
#include <Awesomium\BitmapSurface.h>

#include "other.h"
#include "detours\detours.h"
bool quit = false;
bool isDirty = false;
bool loadPage = false;
bool debug = false;
char pageURL[4096];
char texBuffer[512*512*4];
HANDLE thread;
HANDLE blockland;
typedef int(*SwapBuffersFn)();
MologieDetours::Detour<SwapBuffersFn> *detour_SwapBuffers;
Awesomium::WebCore* wCore;
Awesomium::WebView* wView;
Awesomium::WebSession* wSession;

void ts_AWS_dumpTextures(SimObject* object, int argc, const char** argv) {
	TextureObject* texture;
	unsigned int count = 0;
	int width, height, format, red, green, blue, alpha;
	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		count++;
		glBindTexture(GL_TEXTURE_2D, texture->texGLName);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &red);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &green);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blue);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alpha);
		Printf("\nTexture [%u]: %s\n"
			"  OpenGL ID: %u\n"
			"  GL_TEXTURE_WIDTH: %u\n"
			"  GL_TEXTURE_HEIGHT: %u\n"
			"  GL_TEXTURE_INTERNAL_FORMAT: %u\n"
			"  GL_TEXTURE_RED_SIZE: %u\n"
			"  GL_TEXTURE_GREEN_SIZE: %u\n"
			"  GL_TEXTURE_BLUE_SIZE: %u\n"
			"  GL_TEXTURE_ALPHA_SIZE: %u\n",
			count, texture->texFileName, texture->texGLName, width, height, format, red, green, blue, alpha);
	}
}
void ts_AWS_debug(SimObject*, int, const char**) {
	debug = !debug;
}
int __fastcall hook_SwapBuffers() {
	int a = detour_SwapBuffers->GetOriginalFunction()();
	if(debug)
		Printf("textureID: %u, isDirty: %s, loadPage: %s", textureID, isDirty ? "true":"false", loadPage ? "true":"false");
	if (textureID != 0 && isDirty) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer);
		isDirty = false;
	}
	return a;
}
void ts_AWS_LoadUrl(SimObject* object, int argc, const char** argv) {
	strncpy_s(pageURL, 4096, argv[1], 4095);
	loadPage = true;
}
void ts_AWS_setTextureID(SimObject* object, int argc, const char** argv) {
	textureID = atoi(argv[1]);
	Printf("AWS: Assigned textureID to %u.\r\n%s <<<", atoi(argv[1]), argv[1]);
}
DWORD WINAPI doStuff(LPVOID lpParam) {
	Awesomium::WebConfig wConfig;
	wConfig.log_level = Awesomium::kLogLevel_Verbose;
	wConfig.log_path = Awesomium::WSLit("./Awesomium/Output.log");
	wCore = Awesomium::WebCore::Initialize(wConfig);
	wSession = wCore->CreateWebSession(Awesomium::WSLit("./Awesomium/Session/"), Awesomium::WebPreferences());
	wView = wCore->CreateWebView(512, 512, wSession, Awesomium::kWebViewType_Offscreen);
	Awesomium::BitmapSurface* surface = (Awesomium::BitmapSurface*)(wView->surface());
	while (!quit) {
		wCore->Update();
		if (loadPage) {
			wView->LoadURL(Awesomium::WebURL(Awesomium::WSLit(pageURL)));
			loadPage = false;
		}
		if (surface == NULL) {
			surface = (Awesomium::BitmapSurface*)(wView->surface());
		}
		else {
			if (surface->is_dirty()) {
				surface->CopyTo((unsigned char*)texBuffer, 512 * 4, 4, true, false);
				isDirty = true;
			}
		}
	}
	return true;
}

int init() {
	//
	if (!torque_init())
		return false;
	Printf("Awesomium Hook attached.");
	ConsoleFunction(NULL, "AWS_dumpTextures", ts_AWS_dumpTextures, "() - Dumps all of the OpenGL textures.", 1, 1);
	ConsoleFunction(NULL, "AWS_LoadUrl", ts_AWS_LoadUrl, "(String URL) - Loads a URL into the Awesomium Web View.", 2, 2);
	ConsoleFunction(NULL, "AWS_setTextureID", ts_AWS_setTextureID, "(int textureID) - Sets the ID of the texture for the web view to use.", 2, 2);
	ConsoleFunction(NULL, "AWS_debug", ts_AWS_debug, "() - Toggles debugging.", 1, 1);
	int a = 0;
	Printf("Functions declared.");
	detour_SwapBuffers = new MologieDetours::Detour<SwapBuffersFn>((SwapBuffersFn)0x4237D0, (SwapBuffersFn)hook_SwapBuffers);
	Printf("Passed detour.");
	blockland = GetCurrentThread();
	thread = CreateThread(NULL, 0, doStuff, (void*)a, 0, NULL);
	return true;
}
int deinit() {
	quit = true;
	if (detour_SwapBuffers != NULL)
		delete detour_SwapBuffers;
	WaitForSingleObject(thread, INFINITY);
	CloseHandle(thread);
	wView->Destroy();
	wSession->Release();
	wCore->Shutdown();
	return true;
}
int __stdcall DllMain(HINSTANCE h, unsigned long reason, void* reserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		return init();
	case DLL_PROCESS_DETACH:
		return deinit();
	default:
		return true;
	}
}
extern "C" void __declspec(dllexport) __cdecl placeholder(void) {}