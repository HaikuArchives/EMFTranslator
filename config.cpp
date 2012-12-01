/*
 * config.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <TranslatorFormats.h>
#include <TranslatorAddOn.h>
#include <Message.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
#include <StringView.h>
#include <TextControl.h>

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "config.h"
#include "dbgmsg.h"

char  translatorName[]  = "EMF Translator";
char  translatorInfo[]  = "Enhanced Metafile format translator";
int32 translatorVersion = 6; // format is revision + minor * 10 + major * 100

emf_settings g_settings;
BLocker g_settings_lock("EMF settings lock");
const char *SETTING_FILENAME = "EMFTranslator_Settings";

emf_settings::emf_settings()
{
	DBGMSG(("> emf_settings::emf_settings\n"));

	max_width  = 800;
	max_height = 600;
	settings_touched = false;

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path)) {
//		path.SetTo("/tmp");
	}
	path.Append(SETTING_FILENAME);
	FILE *f = fopen(path.Path(), "r");
	// parse text settings file -- this should be a library...
	if (f) {
		char line[1024];
		char name[32];
		char * ptr;
		while (true) {
			line[0] = 0;
			fgets(line, 1024, f);
			if (!line[0]) {
				break;
			}
			// remember: line ends with \n, so printf()s don't have to
			ptr = line;
			while (isspace(*ptr)) {
				ptr++;
			}
			if (*ptr == '#' || !*ptr) {	/* comment or blank */
				continue;
			}
			if (sscanf(ptr, "%31[a-zA-Z_0-9] =", name) != 1) {
				fprintf(stderr, "unknown %s settings line: %s", translatorName, line);
			} else {
				if (!strcmp(name, "max_width")) {
					while (*ptr != '=') {
						ptr++;
					}
					ptr++;
					if (sscanf(ptr, "%d", (int*)&max_width) != 1) {
						fprintf(stderr, "illegal width in %s settings: %s", translatorName, ptr);
					}
				} else if (!strcmp(name, "max_height")) {
					while (*ptr != '=') {
						ptr++;
					}
					ptr++;
					if (sscanf(ptr, "%d", (int*)&max_height) != 1) {
						fprintf(stderr, "illegal width in %s settings: %s", translatorName, ptr);
					}
				} else {
					fprintf(stderr, "unknown %s setting: %s", translatorName, line);
				}
			}
		}
		fclose(f);
	}

	DBGMSG(("  max_width = %d, max_height = %d\n", max_width, max_height));
	DBGMSG(("< emf_settings::emf_settings\n"));
}

emf_settings::~emf_settings()
{
	DBGMSG(("> emf_settings::~emf_settings\n"));

	/*	No need writing settings if there aren't any	*/
	if (g_settings.settings_touched) {
		BPath path;
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &path)) {
//			path.SetTo("/tmp");
		}
		path.Append(SETTING_FILENAME);
		FILE *f = fopen(path.Path(), "w");
		if (f) {
			fprintf(f, "# %s settings version %ld.%ld.%ld\n",
				translatorName,
				translatorVersion / 100,
				(translatorVersion / 10) % 10,
				translatorVersion % 10);
			fprintf(f, "max_width  = %d\n", max_width);
			fprintf(f, "max_height = %d\n", max_height);
			fclose(f);
		}
	}

	DBGMSG(("< emf_settings::~emf_settings\n"));
};

void emf_settings::SetSettings(int width, int height)
{
	DBGMSG(("> emf_settings::SetSettings\n"));

	g_settings_lock.Lock();
	max_width  = width;
	max_height = height;
	settings_touched = true;
	DBGMSG(("  max_width = %d, max_height = %d\n", max_width, max_height));
	g_settings_lock.Unlock();

	DBGMSG(("< emf_settings::SetSettings\n"));
}

void emf_settings::GetSettings(int *width, int *height) const
{
	DBGMSG(("> emf_settings::GetSettings\n"));

	g_settings_lock.Lock();
	*width  = max_width;
	*height = max_height;
	DBGMSG(("  max_width = %d, max_height = %d\n", max_width, max_height));
	g_settings_lock.Unlock();

	DBGMSG(("< emf_settings::GetSettings\n"));
}

class EMFView : public BView {
public:
	EMFView(const BRect &frame, const char *name, uint32 resize, uint32 flags);
	virtual void AllAttached();
	virtual void AllDetached();
	void SetSettings(BMessage *config);
private:
	BTextControl *max_width;
	BTextControl *max_height;
};

EMFView::EMFView(const BRect &frame, const char *name, uint32 resize, uint32 flags)
	: BView(frame, name, resize, flags)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	char str[64];
	sprintf(str, "%s %ld.%ld.%ld",
		translatorName,
		translatorVersion / 100,
		(translatorVersion / 10) % 10,
		translatorVersion % 10);

	BStringView *s = new BStringView(BRect(10, 10, 140, 33), "", str);
	s->SetFont(be_bold_font);
	AddChild(s);

	max_width  = new BTextControl(BRect(10, 50, 140, 70), "", "Max Width:", "", NULL);
	AddChild(max_width);

	max_height = new BTextControl(BRect(10, 74, 140, 94), "", "Max Height:", "", NULL);
	AddChild(max_height);
}

void EMFView::AllAttached()
{
	DBGMSG(("> EMFView::AllAttached\n"));

	BView::AllAttached();
	int width, height;
	g_settings.GetSettings(&width, &height);
	char s[64];
	sprintf(s, "%d", width);
	max_width->SetText(s);
	sprintf(s, "%d", height);
	max_height->SetText(s);

	DBGMSG(("< EMFView::AllAttached\n"));
}

void EMFView::AllDetached()
{
	DBGMSG(("> EMFView::AllDetached\n"));

	g_settings.SetSettings(atoi(max_width->Text()), atoi(max_height->Text()));
	BView::AllDetached();

	DBGMSG(("< EMFView::AllDetached\n"));
}

void EMFView::SetSettings(BMessage *config)
{
	DBGMSG(("> EMFView::SetSettings\n"));

	if (config->HasRect(B_TRANSLATOR_EXT_BITMAP_RECT)) {
		BRect rect = config->FindRect(B_TRANSLATOR_EXT_BITMAP_RECT);
		g_settings.SetSettings(rect.IntegerWidth(), rect.IntegerHeight());
	}

	DBGMSG(("< EMFView::SetSettings\n"));
}

status_t MakeConfig(BMessage *config, BView **outView, BRect *outExtent)
{
	DBGMSG(("> MakeConfig\n"));

	EMFView *v = new EMFView(BRect(0, 0, 200, 100), "EMFTranslator Settings", B_FOLLOW_ALL, B_WILL_DRAW);
	*outView = v;
	*outExtent = v->Bounds();
	if (config) {
		v->SetSettings(config);
	}

	DBGMSG(("< MakeConfig\n"));
	return B_OK;
}

status_t GetConfigMessage(BMessage *config)
{
	DBGMSG(("> GetConfigMessage\n"));

	int max_width, max_height;
	g_settings.GetSettings(&max_width, &max_height);
	BRect rect(0.0f, 0.0f, max_width, max_height);

	status_t err = B_OK;
	config->RemoveName(B_TRANSLATOR_EXT_BITMAP_RECT);
	err = config->AddRect(B_TRANSLATOR_EXT_BITMAP_RECT, rect);

	DBGMSG(("< GetConfigMessage\n"));
	return err;
}
