#pragma once
#include "torque.h"
#include <gl/GL.h>
#include <Awesomium\Surface.h>
#include <Awesomium\BitmapSurface.h>

static int                    textureID = 0;
struct TextureObject
{
	TextureObject *next;
	TextureObject *prev;
	TextureObject *hashNext;
	unsigned int texGLName;
	unsigned int smallTexGLName;
	const char *texFileName;
	DWORD *type_GBitmap_bitmap;
	unsigned int texWidth;
	unsigned int texHeight;
	unsigned int bitmapWidth;
	unsigned int bitmapHeight;
	unsigned int downloadedWidth;
	unsigned int downloadedHeight;
	unsigned int enum_TextureHandleType_type;
	bool filterNearest;
	bool clamp;
	bool holding;
	int refCount;
};
#define AWSHOOK_EVENT_NA 0
#define AWSHOOK_EVENT_KEYBOARD 1
#define AWSHOOK_EVENT_MOUSE 2
enum KMEType { NA, Keyboard, Mouse, Scroll};
struct KeyMouseEvent {
	
	KMEType event;
	union {
		struct {
			unsigned int mX, mY, mBtn;
		};
		struct {
			unsigned int kChar;
			union {
				unsigned int kFlags;
				struct {
					unsigned int kShift : 1;
					unsigned int kCtrl : 1;
					unsigned int kAlt : 1;
					unsigned int kMeta : 1;
					unsigned int kKeypad : 1;
					unsigned int kAutorepeat : 1;
				};
			};
			unsigned int kExcess;
		};
		struct {
			unsigned int sX, sY, sExcess;
		};
		unsigned long long data;
	};
};