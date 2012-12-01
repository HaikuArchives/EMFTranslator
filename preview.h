/*
 * preview.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#define USE_PREVIEW_FOR_DEBUG

#ifdef USE_PREVIEW_FOR_DEBUG
#ifndef __PREVIEW_H
#define __PREVIEW_H

#include <Window.h>

class PreviewWindow : public BWindow {
public:
	PreviewWindow(BRect, const char *, BBitmap *);
	PreviewWindow(BRect, const char *, BPicture *);
	virtual bool QuitRequested();
	int Go();

private:
	long semaphore;
};

#endif	/* __PREVIEW_H */
#endif	/* USE_PREVIEW_FOR_DEBUG */
