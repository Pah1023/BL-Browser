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
bool isInit = false;
char pageURL[4096];
char texBuffer[1024 * 1024 * 4];
char texBufferB[512 * 512 * 4];
char texBufferC[256 * 256 * 4];
HANDLE thread;
HANDLE blockland;
typedef int(*intFn)();
MologieDetours::Detour<intFn> *detour_SwapBuffers;
MologieDetours::Detour<intFn> *detour_attachOpenGL;
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
	if (debug) {
		Printf("textureID: %u, isDirty: %s, loadPage: %s", textureID, isDirty ? "true" : "false", loadPage ? "true" : "false");
		
	}
	if (textureID != 0 && isDirty) {

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer);
		glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 512, 512, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBufferB);
		glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 256, 256, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBufferC);
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
void ts_AWS_clientCmdLoadURL(SimObject* object, int argc, const char** argv) {
	SetGlobalVariable("AWS::URL", argv[1]);	
}
/* Causes crash, unsure why.
void ts_AWS_bindTexture(SimObject* object, int argc, const char** argv) {
	TextureObject* texture;
	unsigned int count = 0;
	int width, height, format, red, green, blue, alpha;
	const char* string = "Add-Ons/Print_Screen_Cinema/prints/Cinema.png";
	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		if (strcmp(texture->texFileName, string) == 0) {
			textureID = texture->texGLName;
			Printf("Found textureID; %u", texture->texGLName);
			break;
		}
	}
}
*/
DWORD WINAPI doStuff(LPVOID lpParam) {
	Awesomium::WebConfig wConfig;
	wConfig.log_level = Awesomium::kLogLevel_Verbose;
	wConfig.log_path = Awesomium::WSLit("./Awesomium/Output.log");
	wCore = Awesomium::WebCore::Initialize(wConfig);
	Awesomium::WebPreferences wPreferences;
	wPreferences.allow_scripts_to_open_windows = false;
	wPreferences.allow_scripts_to_close_windows = false;
	wPreferences.enable_app_cache = wPreferences.enable_databases = wPreferences.enable_web_gl = true;
	wPreferences.enable_dart = 1;
	wSession = wCore->CreateWebSession(Awesomium::WSLit("./Awesomium/Session/"), wPreferences);
	wView = wCore->CreateWebView(1024, 768, wSession, Awesomium::kWebViewType_Offscreen);
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
				memcpy(texBuffer, surface->buffer(), 1024 * 768 * 4);
				// There's probably a better way to do this, works for now at little to no performance hit.
				for (int x = 0; x < 512; x++) {
					for (int y = 0; y < 512; y++) {
						for (int z = 0; z < 4; z++) {
							texBufferB[x * 512 * 4 + y * 4 + z] = texBuffer[x * 8192 + y * 8 + z];
							if (x & 1 && y & 1) {
								texBufferC[(x >> 1) * 256 * 4 + (y >> 1) * 4 + z] = texBuffer[x * 8192 + y * 8 + z];
							}
						}
					}
				}
				surface->set_is_dirty(false);
				isDirty = true;
			}
		}
	}
	return true;
}

void ts_AWS_scrollWheel(SimObject* obj, int argc, const char** argv) {
	/* This won't work, it has to be on the same thread.
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	wView->InjectMouseWheel(x, y);
	*/
}
void firstRun() {
	ConsoleFunction(NULL, "AWS_dumpTextures", ts_AWS_dumpTextures, "() - Dumps all of the OpenGL textures.", 1, 1);
	ConsoleFunction(NULL, "AWS_LoadUrl", ts_AWS_LoadUrl, "(String URL) - Loads a URL into the Awesomium Web View.", 2, 2);
	ConsoleFunction(NULL, "AWS_setTextureID", ts_AWS_setTextureID, "(int textureID) - Sets the ID of the texture for the web view to use.", 2, 2);
	ConsoleFunction(NULL, "AWS_debug", ts_AWS_debug, "() - Toggles debugging.", 1, 1);
	ConsoleFunction(NULL, "AWS_scrollWheel", ts_AWS_scrollWheel,"(int vert, int horiz) - sends a mouse wheel event.", 3, 3);
	// ConsoleFunction(NULL, "AWS_bindTexture", ts_AWS_bindTexture, "() - Binds the cinema texture.", 1, 1); -- See above.
	Printf("Functions declared.");
	detour_SwapBuffers = new MologieDetours::Detour<intFn>((intFn)0x4237D0, (intFn)hook_SwapBuffers);
	Printf("Passed detour.");
	blockland = GetCurrentThread();
	thread = CreateThread(NULL, 0, doStuff, 0, 0, NULL);
	isInit = true;
}

int __fastcall hook_attachOpenGL() {
	bool ret = detour_attachOpenGL->GetOriginalFunction()();
	if (!isInit) firstRun();
	return ret;
}
int init() {
	if (!torque_init())
		return false;
	if (atoi(GetGlobalVariable("pref::Video::preferOpenGL")) == 1)
		firstRun();
	else
		detour_attachOpenGL = new MologieDetours::Detour<intFn>((intFn)0x403590, (intFn)hook_attachOpenGL);
	return true;
}
int deinit() {
	quit = true;
	isInit = false;
	if (detour_SwapBuffers != NULL)
		delete detour_SwapBuffers;
	if (detour_attachOpenGL != NULL)
		delete detour_attachOpenGL;
	WaitForSingleObject(thread, INFINITE);
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