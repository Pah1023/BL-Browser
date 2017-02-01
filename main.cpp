#pragma once
#include <Windows.h>
#include <AclAPI.h>
#include "gl.h"
#include "torque.h"
#include <Awesomium\WebCore.h>
#include <Awesomium\STLHelpers.h>
#include <Awesomium\BitmapSurface.h>

#include "other.h"
#include "detours\detours.h"
#define AWSHOOK_VERS 0.20
struct {
	char quit : 1;
	char isDirty : 1;
	char loadPage : 1;
	char debug : 1;
	char isInit : 1;

} flags;
char pageURL[4096];
char texBuffer_0[1024 * 1024 * 4];
char texBuffer_1[512 * 512 * 4];
char texBuffer_2[256 * 256 * 4];
char texBuffer_3[128 * 128 * 4];
char texBuffer_4[64 * 64 * 4];
char texBuffer_5[32 * 32 * 4];
char texBuffer_6[16 * 16 * 4];

HANDLE thread;
HANDLE blockland;
HANDLE event;
typedef int(*intFn)();
MologieDetours::Detour<intFn> *detour_SwapBuffers;
MologieDetours::Detour<intFn> *detour_attachOpenGL;
Awesomium::WebCore* wCore;
Awesomium::WebView* wView;
Awesomium::WebSession* wSession;
KeyMouseEvent eventStack[512];
unsigned int eventStackIndex = 0;

void ts_AWS_dumpTextures(SimObject* object, int argc, const char** argv) {
	TextureObject* texture;
	unsigned int count = 0;
	int width, height, format, red, green, blue, alpha;
	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		count++;
		AWS_glBindTexture(GL_TEXTURE_2D, texture->texGLName);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &red);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &green);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blue);
		AWS_glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alpha);
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
	flags.debug = !flags.debug;
}
int __fastcall hook_SwapBuffers() {
	int a = detour_SwapBuffers->GetOriginalFunction()();
	if (flags.debug) {
		Printf("textureID: %u, isDirty: %s, loadPage: %s", textureID, flags.isDirty ? "true" : "false", flags.loadPage ? "true" : "false");
	}
	SetEvent(event);
	if (textureID != 0 && flags.isDirty) {
		AWS_glBindTexture(GL_TEXTURE_2D, textureID);
		AWS_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 1024, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_0);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 512, 512, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_1);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 256, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_2);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 128, 128, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_3);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, 64, 64, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_4);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, 32, 32, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_5);
			AWS_glTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, 16, 16, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer_6);
		flags.isDirty = false;
	}
	return a;
}
void ts_AWS_LoadUrl(SimObject* object, int argc, const char** argv) {
	strncpy_s(pageURL, 4096, argv[1], 4095);
	flags.loadPage = true;
}
void ts_AWS_setTextureID(SimObject* object, int argc, const char** argv) {
	textureID = atoi(argv[1]);
	Printf("AWS: Assigned textureID to %u.\r\n%s <<<", atoi(argv[1]), argv[1]);
}
void ts_AWS_bindTexture(SimObject* object, int argc, const char** argv) {
	TextureObject* texture;
	const char* string = "Add-Ons/Print_Screen_Cinema/prints/Cinema.png";
	textureID = 0;
	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		if (texture->texFileName != NULL && _stricmp(texture->texFileName, string) == 0) {
			textureID = texture->texGLName;
			Printf("Found textureID; %u", texture->texGLName);
			break;
		}
	}
}

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
	wView->Focus();
	while (!flags.quit) {
		WaitForSingleObject(event, 100);
		ResetEvent(event);
		while (eventStackIndex) {
			eventStackIndex--;
			switch (eventStack[eventStackIndex].event) {
			case Keyboard: {
				Awesomium::WebKeyboardEvent kEvent;
				wView->Focus();
				kEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyDown;
				char* buf = new char[20];
				kEvent.text[0] = eventStack[eventStackIndex].kChar;
				kEvent.unmodified_text[0] = tolower(eventStack[eventStackIndex].kChar);
				kEvent.modifiers = (eventStack[eventStackIndex].kShift ? 1:0);
				kEvent.virtual_key_code = VkKeyScan(kEvent.unmodified_text[0]);
				Awesomium::GetKeyIdentifierFromVirtualKeyCode(kEvent.virtual_key_code, &buf);
				strcpy_s(kEvent.key_identifier, buf);
				wView->InjectKeyboardEvent(kEvent);
				kEvent.type = Awesomium::WebKeyboardEvent::kTypeChar;
				wView->InjectKeyboardEvent(kEvent);
				kEvent.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
				wView->InjectKeyboardEvent(kEvent);
				break;
			}
			case Scroll: {
				wView->InjectMouseWheel(eventStack[eventStackIndex].sX, eventStack[eventStackIndex].sY);
				break;
			}
			case Mouse: {
				wView->InjectMouseMove(eventStack[eventStackIndex].mX, eventStack[eventStackIndex].mY);
				unsigned int btn = eventStack[eventStackIndex].mBtn;
				Awesomium::MouseButton b = (btn == 0 ? Awesomium::kMouseButton_Left : (btn == 1 ? Awesomium::kMouseButton_Right : Awesomium::kMouseButton_Middle));
				if (btn < 3) {
					wView->InjectMouseDown(b);
					wView->InjectMouseUp(b);
				}
				break;
			}
			case NA:
			default:
				break;
			}
		}
		wCore->Update();
		
		if (flags.loadPage) {
			wView->LoadURL(Awesomium::WebURL(Awesomium::WSLit(pageURL)));
			flags.loadPage = false;
		}
		if (surface == NULL) {
			surface = (Awesomium::BitmapSurface*)(wView->surface());
		}
		else {
			if (surface->is_dirty()) {
				memcpy(texBuffer_0, surface->buffer(), 1024 * 768 * 4);
				// There's probably a better way to do this, works for now at little to no performance hit.
				for (int x = 0; x < 512; x++) {
					for (int y = 0; y < 512; y++) {
						for (int z = 0; z < 4; z++) {
							texBuffer_1[x * 512 * 4 + y * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
							if (x & 1 && y & 1) {
								texBuffer_2[(x >> 1) * 256 * 4 + (y >> 1) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
								if (x & 2 && y & 2) {
									texBuffer_3[(x >> 2) * 128 * 4 + (y >> 2) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
									if (x & 4 && y & 4) {
										texBuffer_4[(x >> 3) * 64 * 4 + (y >> 3) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
										if(x & 8 && y & 8) {
											texBuffer_5[(x >> 4) * 32 * 4 + (y >> 4) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
											if (x & 16 && y & 16) {
												texBuffer_6[(x >> 5) * 16 * 4 + (y >> 5) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
												/*if (x & 32 && y & 32) {
													texBuffer_7[(x >> 6) * 8 * 4 + (y >> 6) * 4 + z] = texBuffer_0[x * 8192 + y * 8 + z];
												}*/
											}
										}
									}
								}
							}
						}
					}
				}
				surface->set_is_dirty(false);
				flags.isDirty = true;
			}
		}
	}
	Printf("Quit event received, destroying view.");
	wView->Destroy();
	Printf("Destroying session.");
	wSession->Release();
	Printf("Shutting diown.");
	wCore->Shutdown();
	Printf("Returning.");
	SetEvent(event);
	return 0;
}

void ts_AWS_scrollWheel(SimObject* obj, int argc, const char** argv) {
	eventStack[eventStackIndex].event = Scroll;
	eventStack[eventStackIndex].sX = atoi(argv[1]);
	eventStack[eventStackIndex].sY = atoi(argv[2]);
	eventStackIndex++;
	/* This won't work, it has to be on the same thread.
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	wView->InjectMouseWheel(x, y);
	*/
}
void ts_AWS_MouseEvent(SimObject* object, int argc, const char** argv) {
	eventStack[eventStackIndex].event = Mouse;
	eventStack[eventStackIndex].mX = atoi(argv[1]);
	eventStack[eventStackIndex].mY = atoi(argv[2]);
	eventStack[eventStackIndex].mBtn = atoi(argv[3]);
	eventStackIndex++;
}
void ts_AWS_KeyboardEvent(SimObject* object, int argc, const char** argv) {
	eventStack[eventStackIndex].event = Keyboard;
	eventStack[eventStackIndex].kChar = argv[1][0];
	eventStack[eventStackIndex].kFlags = 0;
	eventStack[eventStackIndex].kShift = atoi(argv[2]);
	eventStackIndex++;
}
void firstRun() {
	ConsoleFunction(NULL, "AWS_dumpTextures", ts_AWS_dumpTextures, "() - Dumps all of the OpenGL textures.", 1, 1);
	ConsoleFunction(NULL, "AWS_LoadUrl", ts_AWS_LoadUrl, "(String URL) - Loads a URL into the Awesomium Web View.", 2, 2);
	ConsoleFunction(NULL, "AWS_setTextureID", ts_AWS_setTextureID, "(int textureID) - Sets the ID of the texture for the web view to use.", 2, 2);
	ConsoleFunction(NULL, "AWS_debug", ts_AWS_debug, "() - Toggles debugging.", 1, 1);
	ConsoleFunction(NULL, "AWS_scrollWheel", ts_AWS_scrollWheel,"(int vert, int horiz) - sends a mouse wheel event.", 3, 3);
	ConsoleFunction(NULL, "AWS_bindTexture", ts_AWS_bindTexture, "() - Binds the cinema texture.", 1, 1);
	ConsoleFunction(NULL, "AWS_MouseEvent", ts_AWS_MouseEvent, "(int x, int y, int btn) - moves mouse to position and clicks, value of 3 or higher is ignored.", 4, 4);
	ConsoleFunction(NULL, "AWS_KeyboardEvent", ts_AWS_KeyboardEvent, "(char character, bool shift) - sends keyboard character, bool has to be 0/1.", 3, 3);
	initGL();
	// Still keeping these functions for support on already made server codes.
	Eval("function clientCmdAWS_LoadYoutube(%id){AWS_LoadUrl(\"https://Pah1023.github.io/index.html?videoid=\"@%id);}");
	Eval("function clientCmdAWS_PlayYoutube(%id, %start){AWS_LoadUrl(\"https://Pah1023.github.io/index.html?videoid=\"@%id@\"&start=\"@%start);}");
	Eval("function clientCmdAWS_LoadURL(%url){AWS_LoadUrl(%url);}");
	Eval("function clientCmdAWS_ClearLink(){AWS_LoadUrl(\"\");}");
	Eval("package AWSPackage{function clientCmdMissionStartPhase3(%a0, %a1, %a2){Parent::clientCmdMissionStartPhase3(%a0, %a1, %a2);AWS_BindTexture();}function flushTextureCache(){Parent::flushTextureCache();schedule(100,0,AWS_BindTexture);}function disconnect(%b){AWS_LoadUrl(\"\");Parent::disconnect(%b);}}; activatePackage(\"AWSPackage\");");
	Eval("function clientCmdAWS_MouseEvent(%x,%y,%b){AWS_MouseEvent(%x,%y,%b);}");
	Eval("function clientCmdAWS_Version(){commandToServer('AWS_Version', 0, 2, 0);}");
	Printf("Functions declared.");
	detour_SwapBuffers = new MologieDetours::Detour<intFn>((intFn)0x4237D0, (intFn)hook_SwapBuffers);
	Printf("Passed detour.");
	blockland = GetCurrentThread();
	event = CreateEvent(NULL, TRUE, FALSE, "blawsevent");
	thread = CreateThread(NULL, 0, doStuff, 0, 0, NULL);
	flags.isInit = true;
}

int __fastcall hook_attachOpenGL() {
	bool ret = detour_attachOpenGL->GetOriginalFunction()();
	if (!flags.isInit) firstRun();
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
	flags.quit = true;
	if (detour_SwapBuffers != NULL)
		delete detour_SwapBuffers;
	if (detour_attachOpenGL != NULL)
		delete detour_attachOpenGL;
	delete[] texBuffer_0;
	delete[] texBuffer_1;
	delete[] texBuffer_2;
	delete[] texBuffer_3;
	delete[] texBuffer_4;
	delete[] texBuffer_5;
	delete[] texBuffer_6;
	WaitForSingleObject(event, 750);
	TerminateThread(thread, 0);
	flags.isInit = false;
	CloseHandle(thread);
	CloseHandle(event);
	
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