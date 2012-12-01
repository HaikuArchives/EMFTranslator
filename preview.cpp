/*
 * preview.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <View.h>
#include "preview.h"

#ifdef USE_PREVIEW_FOR_DEBUG

class PreviewView : public BView {
public:
	PreviewView(BRect, BPicture *);
	PreviewView(BRect, BBitmap *);
	void Draw(BRect);
private:
	BPicture *picture;
	BBitmap *bitmap;
};

PreviewView::PreviewView(BRect a_frame, BPicture *a_picture)
	: BView(a_frame, "preview", B_FOLLOW_ALL, B_WILL_DRAW)
{
	picture = a_picture;
	bitmap  = NULL;
}

PreviewView::PreviewView(BRect a_frame, BBitmap *a_bitmap)
	: BView(a_frame, "preview", B_FOLLOW_ALL, B_WILL_DRAW)
{
	picture = NULL;
	bitmap  = a_bitmap;
}

void PreviewView::Draw(BRect)
{
	if (picture) {
		BPoint pt(0.0, 0.0);
		DrawPicture(picture, pt);
	} else if (bitmap) {
		DrawBitmap(bitmap);
	}
} 

PreviewWindow::PreviewWindow(BRect frame, const char *title, BPicture *a_picture)
	: BWindow(frame, title, B_TITLED_WINDOW, 0)
{
	AddChild(new PreviewView(Bounds(), a_picture));
	semaphore = create_sem(0, "PreviewSem");
}

PreviewWindow::PreviewWindow(BRect frame, const char *title, BBitmap *bitmap)
	: BWindow(frame, title, B_TITLED_WINDOW, 0)
{
	AddChild(new PreviewView(Bounds(), bitmap));
	semaphore = create_sem(0, "PreviewSem");
}

bool PreviewWindow::QuitRequested()
{
	release_sem(semaphore);
	return true;
}

int PreviewWindow::Go()
{
	Show();
	acquire_sem(semaphore);
	delete_sem(semaphore);
	Lock();
	Quit();
	return 0;
}

#endif	/* USE_PREVIEW_FOR_DEBUG */
