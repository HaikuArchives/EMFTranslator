/*
 * entries.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <TranslatorFormats.h>
#include <TranslatorAddOn.h>
#include <GraphicsDefs.h>
#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>

#include <cstdio>
#include <string.h>

#include "translator.h"
#include "dbgmsg.h"

#define EMF_TYPE 'EMF '

translation_format inputFormats[] = {
//	{ B_TRANSLATOR_BITMAP,  B_TRANSLATOR_BITMAP,  0.5, 0.5, "image/x-be-bitmap",    "Be Bitmap Format" },
//	{ B_TRANSLATOR_PICTURE, B_TRANSLATOR_PICTURE, 0.5, 0.5, "picture/x-be-picture", "Be Picture Format" },
	{ EMF_TYPE,             B_TRANSLATOR_PICTURE, 0.5, 0.5, "picture/x-emf",        "Enhanced Metafile Format" },
	{ 0,                    0,                    0.0, 0.0, "",                     "" }
};

translation_format outputFormats[] = {
	{ B_TRANSLATOR_BITMAP,  B_TRANSLATOR_BITMAP,  0.5, 0.5, "image/x-be-bitmap",    "Be Bitmap Format" },
	{ B_TRANSLATOR_PICTURE, B_TRANSLATOR_PICTURE, 0.5, 0.5, "picture/x-be-picture", "Be Picture Format" },
//	{ EMF_TYPE,             B_TRANSLATOR_PICTURE, 0.5, 0.5, "picture/x-emf",        "Enhanced Metafile Format" },
	{ 0,                    0,                    0.0, 0.0, "",                     "" }
};

status_t read_bits_header(BDataIO *io, int skipped)
{
	/* read the rest of a possible B_TRANSLATOR_BITMAP header */

	if (skipped < 0 || skipped > 4)
		return B_NO_TRANSLATOR;

	int rd = sizeof(TranslatorBitmap) - skipped;
	TranslatorBitmap hdr;

	/* pre-initialize magic because we might have skipped part of it already */

	hdr.magic = B_HOST_TO_BENDIAN_INT32(B_TRANSLATOR_BITMAP);
	char *ptr = (char *)&hdr;
	if (io->Read(ptr + skipped, rd) != rd) {
		return B_NO_TRANSLATOR;
	}

	/* swap header values */

	hdr.magic         = B_BENDIAN_TO_HOST_INT32(hdr.magic);
	hdr.bounds.left   = B_BENDIAN_TO_HOST_FLOAT(hdr.bounds.left);
	hdr.bounds.right  = B_BENDIAN_TO_HOST_FLOAT(hdr.bounds.right);
	hdr.bounds.top    = B_BENDIAN_TO_HOST_FLOAT(hdr.bounds.top);
	hdr.bounds.bottom = B_BENDIAN_TO_HOST_FLOAT(hdr.bounds.bottom);
	hdr.rowBytes      = B_BENDIAN_TO_HOST_INT32(hdr.rowBytes);
	hdr.colors        = (color_space)B_BENDIAN_TO_HOST_INT32(hdr.colors);
	hdr.dataSize      = B_BENDIAN_TO_HOST_INT32(hdr.dataSize);

	/* sanity checking */

	if (hdr.magic != B_TRANSLATOR_BITMAP) {
		return B_NO_TRANSLATOR;
	}
	if (hdr.colors & 0xffff0000) {	/* according to <GraphicsDefs.h> this is a reasonable check. */
		return B_NO_TRANSLATOR;
	}
	if (hdr.rowBytes * (hdr.bounds.Height() + 1) > hdr.dataSize) {
		return B_NO_TRANSLATOR;
	}

	return B_OK;
}

status_t check_header(BDataIO *inSource, bool *is_emf)
{
	*is_emf = false;

	/* check B_TRANSLATOR_BITMAP magic */

	char ch[2];
	if (inSource->Read(ch, 2) != 2) {
		return B_NO_TRANSLATOR;
	}

	if (ch[0] == 'b' && ch[1] == 'i') {
		return read_bits_header(inSource, 2);
	}

	/* check EMF magic */

	ENHMETAHEADER emh;
	if (inSource->Read((char *)&emh + 2, sizeof(emh) - 2)) {
		if (emh.dSignature == ENHMETA_SIGNATURE) {
			*is_emf = true;
			return B_OK;
		}
	}

	return B_NO_TRANSLATOR;
}

void dump_translation_format(const translation_format *format)
{
	DBGMSG(("\ttranslation_format::type = %c%c%c%c\n",
		(char)(format->type >> 24),
		(char)(format->type >> 16),
		(char)(format->type >> 8),
		(char)format->type));
	DBGMSG(("\ttranslation_format::group = %c%c%c%c\n",
		(char)(format->group >> 24),
		(char)(format->group >> 16),
		(char)(format->group >> 8),
		(char)format->group));
	DBGMSG(("\ttranslation_format::quality = %f\n", format->quality));
	DBGMSG(("\ttranslation_format::capability = %f\n", format->capability));
	DBGMSG(("\ttranslation_format::MIME = %s\n", format->MIME));
	DBGMSG(("\ttranslation_format::name = %s\n", format->name));
}

status_t Identify(
	BPositionIO *is,
	const translation_format *inFormat,	// can be NULL
	BMessage * /* ioExtension */,			// can be NULL
	translator_info *outInfo,
	uint32 outType)
{
	DBGMSG(("> Identify\n"));

#ifdef DBG
	if (inFormat) {
		DBGMSG(("\tinFormat\n"));
		dump_translation_format(inFormat);
	} else {
		DBGMSG(("\tinFormat is NULL\n"));
	}
	DBGMSG(("\toutType = %c%c%c%c\n",
		(char)(outType >> 24),
		(char)(outType >> 16),
		(char)(outType >> 8),
		(char)outType));
#endif	// DBG

	/* Check that requested format is something we can deal with. */

	bool isMatch = false;

	if (outType == 0) {
		DBGMSG(("\toutType <--- B_TRANSLATOR_BITMAP\n"));
		outType = B_TRANSLATOR_BITMAP;
	}

	for (translation_format *format = outputFormats; format->type; format++) {
		if (outType == format->type) {
			isMatch = true;
			break;
		}
	}

	if (!isMatch) {
		DBGMSG(("< Identify: no match.\n"));
		return B_NO_TRANSLATOR;
	}

	if (inFormat) {
		for (translation_format *format = inputFormats; format->type; format++) {
			if (inFormat->type == format->type) {
				outInfo->type = format->type;
//				outInfo->translator = 0;
				outInfo->quality = format->quality;
				outInfo->capability = format->capability;
				strcpy(outInfo->name, format->name);
				strcpy(outInfo->MIME, format->MIME);
				DBGMSG(("< Identify: found inputFormat.\n"));
				return B_OK;
			}
		}
	}

	/* Check header. */
	bool is_emf;
	status_t err = check_header(is, &is_emf);
	if (err != B_OK) {
		return err;
	}

	/* Stuff info into info struct -- Translation Kit will do "translator" for us. */

	outInfo->group = B_TRANSLATOR_BITMAP;
	outInfo->translator = 0;
	outInfo->quality    = 0.5;	/* no alpha, etc */
	outInfo->capability = 0.5;	/* we're pretty good at PPM reading, though */
	if (is_emf) {
		DBGMSG(("\tis_emf == true\n"));
		outInfo->type = EMF_TYPE;
		strcpy(outInfo->name, "Enhanced Metafile Format");
		strcpy(outInfo->MIME, "picture/x-emf");
	} else {
		DBGMSG(("\tis_emf == false\n"));
		outInfo->type = B_TRANSLATOR_BITMAP;
		strcpy(outInfo->name, "Be Bitmap Format");
		strcpy(outInfo->MIME, "image/x-be-bitmap");	/* this is the MIME type of B_TRANSLATOR_BITMAP */
	}

	DBGMSG(("< Identify\n"));
	return B_OK;
}

status_t easycopy(BPositionIO *is, BPositionIO *os)
{
	const int BUFSIZE = 1024;
	char *buf = new char[BUFSIZE];

	DBGMSG(("> copy\n"));

	if (buf == NULL) {
		DBGMSG(("< copy\n"));
		return B_NO_MEMORY;
	}

	ssize_t len;
	status_t err = B_OK;

	while ((len = is->Read(buf, BUFSIZE)) > 0) {
		if ((len != os->Write(buf, len)) < 0) {
			err = B_NO_TRANSLATOR;
			break;
		}
	}
	delete [] buf;

	DBGMSG(("< copy\n"));
	return err;
}

status_t convert_bitmap_to_picture(BPositionIO *is, BPositionIO *os, BMessage *ioExtension)
{
	status_t err = B_NO_TRANSLATOR;

	DBGMSG(("> convert_bitmap_to_picture\n"));
/*
	BRect rect(0.0f, 0.0f, size.x - 1.0f, size.y - 1.0f);
	BWindow *window = new BWindow(rect, "", B_TITLED_WINDOW, 0);
	view = new BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW);
	window->AddChild(view);
	window->Lock();
	view->BeginPicture(new BPicture);
	BBitmap *bitmap = new BBitmap(rect, B_RGB32);
	bitmap->SetBits(const void *data, int32 length, int32 offset, color_space mode);
	view->DrawBitmap(bitmap);
	delete bitmap;
	BPicture *picture = view->EndPicture();
	picture->Flatten(os);
	delete picture;
	window->Quit();
*/

	DBGMSG(("< convert_bitmap_to_picture\n"));
	return err;
}

status_t convert_bitmap_to_emf(BPositionIO *is, BPositionIO *os, BMessage *ioExtension)
{
	status_t err = B_NO_TRANSLATOR;

	DBGMSG(("> convert_bitmap_to_emf\n"));
	DBGMSG(("< convert_bitmap_to_emf\n"));
	return err;
}

status_t convert_picture_to_bitmap(BPositionIO *is, BPositionIO *os, BMessage *ioExtension)
{
	status_t err = B_NO_TRANSLATOR;

	DBGMSG(("> convert_picture_to_bitmap\n"));
/*
	BRect rect(0.0f, 0.0f, size.x - 1.0f, size.y - 1.0f);
	bitmap = new BBitmap(rect, B_RGB32, true);
	view = new BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW);
	bitmap->AddChild(view);
	view->Window()->Lock();
	view->SetScale(1.0f);
	view->SetHighColor(255, 255, 255);
	view->ConstrainClippingRegion(NULL);
	view->FillRect(view->Bounds());
	BPicture *picture = new BPicture;
	picture->Unflatten(is);
	view->DrawPicture(picture);
	delete picture;
	window->Quit();
*/
	DBGMSG(("< convert_picture_to_bitmap\n"));
	return err;
}

status_t convert_picture_to_emf(BPositionIO *is, BPositionIO *os, BMessage *extension)
{
	status_t err = B_NO_TRANSLATOR;
	DBGMSG(("> convert_picture_to_emf\n"));
	DBGMSG(("< convert_picture_to_emf\n"));
	return err;
}

status_t convert_emf_to_picture(BPositionIO *is, BPositionIO *os, BMessage *config)
{
	DBGMSG(("> convert_emf_to_picture\n"));
	EMFTranslator translator;
	status_t err = translator.ConvertToPicture(is, os, config);
	DBGMSG(("< convert_emf_to_picture\n"));
	return err;
}

status_t convert_emf_to_bitmap(BPositionIO *is, BPositionIO *os, BMessage *config)
{
	DBGMSG(("> convert_emf_to_bitmap\n"));
	EMFTranslator translator;
	status_t err = translator.ConvertToBitmap(is, os, config);
	DBGMSG(("< convert_emf_to_bitmap\n"));
	return err;
}

status_t Translate(
	BPositionIO *is,
	const translator_info *inInfo,
	BMessage *config,	// can be NULL
	uint32 outType,
	BPositionIO *os)
{
	DBGMSG(("> Translate\n"));

	status_t err = B_NO_TRANSLATOR;

	is->Seek(0, SEEK_SET);	/* paranoia */

	if (inInfo->type == outType) {
		err = easycopy(is, os);
	} else {	/* conversion */
		if (inInfo->type == B_TRANSLATOR_BITMAP) {
			if (outType == B_TRANSLATOR_PICTURE) {
				err = convert_bitmap_to_picture(is, os, config);
			} else if (outType == EMF_TYPE) {
				err = convert_bitmap_to_emf(is, os, config);
			}
		} else if (inInfo->type == B_TRANSLATOR_PICTURE) {
			if (outType == B_TRANSLATOR_BITMAP) {
				err = convert_picture_to_bitmap(is, os, config);
			} else if (outType == EMF_TYPE) {
				err = convert_picture_to_emf(is, os, config);
			}
		} else if (inInfo->type == EMF_TYPE) {
			if (outType == B_TRANSLATOR_BITMAP) {
				err = convert_emf_to_bitmap(is, os, config);
			} else if (outType == B_TRANSLATOR_PICTURE) {
				err = convert_emf_to_picture(is, os, config);
			}
		}
	}

	DBGMSG(("< Translate: %d\n", err));
	return err;
}

int main()
{
	return 0;
}
