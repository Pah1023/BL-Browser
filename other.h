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

struct Event {
	unsigned int type = 0;

};
struct KeyboardEvent : public Event {
	unsigned int type = 1;


};
struct MouseEvent : public Event {
	unsigned int type = 2;
};