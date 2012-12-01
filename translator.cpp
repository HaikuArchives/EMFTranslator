/*
 * translator.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <Bitmap.h>
#include <View.h>
#include <Region.h>
#include <Polygon.h>
#include <Shape.h>
#include <UTF8.h>
#include <File.h>
#include <Picture.h>
#include <TranslatorFormats.h>
#include <TranslatorAddOn.h>

#include <cstdio>
#include <algorithm>
#include <climits>

#include "translator.h"
#include "gdicontainer.h"
#include "gdi.h"
#include "config.h"
#include "dbgmsg.h"

#if (!__MWERKS__ || defined(MSIPL_USING_NAMESPACE))
using namespace std;
#else 
#define std
#endif

DC::DC()
{
	memset(this, 0, sizeof(DC));
	map_mode = MM_TEXT;
}

pattern HORIZONTAL = {	/* ----- */
	0xff,	// 11111111
	0x00,	// 00000000
	0x00,	// 00000000
	0x00,	// 00000000
	0x00,	// 00000000
	0x00,	// 00000000
	0x00,	// 00000000
	0x00,	// 00000000
};

pattern VERTICAL = {	/* ||||| */
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80	// 10000000
};

pattern FDIAGONAL = {	/* \\\\\ */
	0x80,	// 10000000
	0x40,	// 01000000
	0x20,	// 00100000
	0x10,	// 00010000
	0x08,	// 00001000
	0x04,	// 00000100
	0x02,	// 00000010
	0x01	// 00000001
};

pattern BDIAGONAL = {	/* ///// */
	0x01,	// 00000001
	0x02,	// 00000010
	0x04,	// 00000100
	0x08,	// 00001000
	0x10,	// 00010000
	0x20,	// 00100000
	0x40,	// 01000000
	0x80	// 10000000
};

pattern CROSS = {	/* +++++ */
	0xff,	// 11111111
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80,	// 10000000
	0x80	// 10000000
};

pattern DIAGCROSS = {	/* xxxxx */
	0x81,	// 10000001
	0x42,	// 01000010
	0x24,	// 00100100
	0x18,	// 00011000
	0x18,	// 00011000
	0x24,	// 00100100
	0x42,	// 01000010
	0x81	// 10000001
};

/*
1 0001
2 0010
3 0011
4 0100
5 0101
6 0110
7 0111
8 1000
9 1001
a 1010
b 1011
c 1100
d 1101
e 1110
f 1111
*/

inline void StrokePolygon(
	BView *view,
	BPoint *pointList,
	int32 numPoints,
	bool isClosed = true,
	pattern aPattern = B_SOLID_HIGH)
{
	if (numPoints == 2) {
		view->StrokeLine(*pointList, *(pointList + 1), aPattern);	/* R4.5 bug? */
	} else {
		view->StrokePolygon(pointList, numPoints, isClosed, aPattern);
	}
}

void convert_unicode_to_utf8(wchar_t *pszSrc, int32 lenSrc, char *pszDst, int32 *lenDst)
{
	const uint16 *pIn = (const uint16 *)pszSrc;
	uint8 *pOut = (uint8 *)pszDst;

	if (lenSrc == 0)
		lenSrc = INT_MAX;

	while (*pIn && lenSrc--) {
		if (*pIn & 0xFF80) {
			if (*pIn & 0xF800) {
				*pOut++ = (uint8)0xE0 | (uint8)((*pIn  & 0xF000) >> 12);
				*pOut++ = (uint8)0x80 | (uint8)((*pIn  & 0x0FC0) >> 6);
				*pOut++ = (uint8)0x80 | (uint8)(*pIn++ & 0x003F);
			} else {
				*pOut++ = (uint8)0xC0 | (uint8)((*pIn  & 0x07C0) >> 6);
				*pOut++ = (uint8)0x80 | (uint8)(*pIn++ & 0x003F);
			}
		} else {
			*pOut++ = (uint8)*pIn++;
		}
	}

	*pOut = 0;
	*lenDst = (int32)pOut - (int32)pszDst;
}

/*
bool preview(BBitmap *a_bitmap)
{
	BRect rc(0.0f, 0.0f, viewportExt.x - 1.0f, viewportExt.y - 1.0f);
	rc.OffsetTo(30.0, 30.0);
	PreviewWindow *preview = new PreviewWindow(rc, "Preview", a_bitmap);
	preview->Go();
	return true;
}
*/

BPoint getMaxSize(BMessage *config)
{
	BPoint size;

	int width, height;
	g_settings.GetSettings(&width, &height);
	size.x = width;
	size.y = height;

	if (config && config->HasRect(B_TRANSLATOR_EXT_BITMAP_RECT)) {
		BRect rect = config->FindRect(B_TRANSLATOR_EXT_BITMAP_RECT);
		size.x = rect.Width();
		size.y = rect.Height();
	}

	return size;
}

status_t EMFTranslator::ConvertToPicture(BPositionIO *is, BPositionIO *os, BMessage *config)
{
	DBGMSG(("> ConvertToPicture\n"));

	if (!EMFHStream(is, &emh)) {
		DBGMSG(("< ConvertToPicture: EMFHStream failed.\n"));
		return B_NO_TRANSLATOR;
	}

	grad = 1.0f;
//	windowExt.x  = 1;
//	windowExt.y  = 1;
	windowOrg.x  = 0;
	windowOrg.y  = 0;

	BPoint MaxSize = getMaxSize(config);
	float g1 = MaxSize.x / MaxSize.y;
	float g2 = (float)(emh.rclFrame.right - emh.rclFrame.left - 1) / (float)(emh.rclFrame.bottom - emh.rclFrame.top - 1);

	if (g1 < g2) {
		viewportExt.x = MaxSize.x;
		viewportExt.y = MaxSize.x / g2;
	} else {
		viewportExt.x = MaxSize.y * g2;
		viewportExt.y = MaxSize.y;
	}

	BRect rect(0.0f, 0.0f, viewportExt.x - 1.0f, viewportExt.y - 1.0f);
	BWindow *window = new BWindow(rect, "", B_TITLED_WINDOW, 0);
	view = new BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW);
	window->AddChild(view);
	window->Lock();
	view->BeginPicture(new BPicture);
	if (!EMFRStream(is, emh.nRecords, emh.nHandles)) {
		DBGMSG(("< ConvertToPicture: EMFRStream failed.\n"));
		return B_NO_TRANSLATOR;
	}
	BPicture *picture = view->EndPicture();
	picture->Flatten(os);
	delete picture;

	DBGMSG(("< ConvertToPicture\n"));
	return B_OK;
}

status_t EMFTranslator::ConvertToBitmap(BPositionIO *is, BPositionIO *os, BMessage *config)
{
	DBGMSG(("> ConvertToBitmap\n"));

	if (!EMFHStream(is, &emh)) {
		DBGMSG(("< ConvertToBitmap: EMFHStream failed.\n"));
		return B_NO_TRANSLATOR;
	}

	grad = 1.0f;
//	windowExt.x  = 1;
//	windowExt.y  = 1;
	windowOrg.x  = 0;
	windowOrg.y  = 0;

	BPoint MaxSize = getMaxSize(config);
	float g1 = MaxSize.x / MaxSize.y;
//	float g2 = (float)(emh.rclFrame.right - emh.rclFrame.left - 1) / (float)(emh.rclFrame.bottom - emh.rclFrame.top - 1);
	float g2 = (float)(emh.rclBounds.right + emh.rclBounds.left) / (float)(emh.rclBounds.bottom + emh.rclBounds.top);

	if (g1 < g2) {
		viewportExt.x = MaxSize.x;
		viewportExt.y = MaxSize.x / g2;
	} else {
		viewportExt.x = MaxSize.y * g2;
		viewportExt.y = MaxSize.y;
	}

	grad = min(viewportExt.x / (float)(emh.rclBounds.right + emh.rclBounds.left) ,
			viewportExt.y / (float)(emh.rclBounds.bottom + emh.rclBounds.top));

	BRect rect(0.0f, 0.0f, viewportExt.x - 1.0f, viewportExt.y - 1.0f);
	BBitmap *bitmap = new BBitmap(rect, B_RGB32, true);
	view = new BView(rect, "", B_FOLLOW_ALL, B_WILL_DRAW);
	bitmap->AddChild(view);
	view->Window()->Lock();
	view->SetHighColor(255, 255, 255);
	view->ConstrainClippingRegion(NULL);
	view->FillRect(view->Bounds());

	if (!EMFRStream(is, emh.nRecords, emh.nHandles)) {
		DBGMSG(("< ConvertToBitmap: EMFRStream failed.\n"));
		return B_NO_TRANSLATOR;
	}

	view->Sync();

	TranslatorBitmap hdr;
	hdr.magic         = B_TRANSLATOR_BITMAP;
	hdr.bounds        = bitmap->Bounds();
	hdr.rowBytes      = bitmap->BytesPerRow();
	hdr.colors        = bitmap->ColorSpace();
	hdr.dataSize      = bitmap->BitsLength();

	hdr.magic         = B_HOST_TO_BENDIAN_INT32(hdr.magic);
	hdr.bounds.left   = B_HOST_TO_BENDIAN_FLOAT(hdr.bounds.left);
	hdr.bounds.right  = B_HOST_TO_BENDIAN_FLOAT(hdr.bounds.right);
	hdr.bounds.top    = B_HOST_TO_BENDIAN_FLOAT(hdr.bounds.top);
	hdr.bounds.bottom = B_HOST_TO_BENDIAN_FLOAT(hdr.bounds.bottom);
	hdr.rowBytes      = B_HOST_TO_BENDIAN_INT32(hdr.rowBytes);
	hdr.colors        = (color_space)B_HOST_TO_BENDIAN_INT32(hdr.colors);
	hdr.dataSize      = B_HOST_TO_BENDIAN_INT32(hdr.dataSize);

	os->Write(&hdr, sizeof(TranslatorBitmap));
	os->Write(bitmap->Bits(), bitmap->BitsLength());
	delete bitmap;

	DBGMSG(("< ConvertToBitmap\n"));
	return B_OK;
}

bool EMFTranslator::EMFHStream(BPositionIO *is, ENHMETAHEADER *pemh)
{
	DBGMSG(("> EMFHStream\n"));

	if (is == NULL || pemh == NULL) {
		DBGMSG(("< EMFHStream: Invalid parameter.\n"));
		return false;
	}

	if (sizeof(ENHMETAHEADER) != is->Read((char *)pemh, sizeof(ENHMETAHEADER))) {
		DBGMSG(("< EMFHStream: Read failed.\n"));
		return false;
	}

	DUMP_ENHMETAHEADER(pemh);

	if (pemh->offDescription > 0) {
		is->Seek(pemh->offDescription + (pemh->nDescription + 1) * sizeof(wchar_t), SEEK_SET);
	}

	DBGMSG(("< EMFHStream\n"));
	return true;
}

bool EMFTranslator::EMFRStream(BPositionIO *is, uint nRecords, uint nHandles)
{
	DBGMSG(("> EMFRStream\n"));

	if (is == NULL || nRecords == 0) {
		DBGMSG(("< EMFRStream: Invalid parameter.\n"));
		return false;
	}

	EMR semr;
	ENHMETARECORD *lemr;
	size_t size;

	nMaxHandles = emh.nHandles;
	container = new GDIContainer(nHandles);
	bool success = true;

	while (nRecords--) {
		if (sizeof(semr) != is->Read((char *)&semr, sizeof(semr))) {
			success = false;
			DBGMSG(("\tRead failed.\n"));
			break;
		}
		if (semr.iType == EMR_EOF) {
			DBGMSG(("\tend of record.\n"));
			break;
		}
		lemr = (ENHMETARECORD *)new char[semr.nSize];
		lemr->iType = semr.iType;
		lemr->nSize = semr.nSize;
		size = semr.nSize - sizeof(semr);
		if ((ssize_t)size != is->Read((char *)lemr->dParm, size)) {
			DBGMSG(("\tRead failed. (lemr->dParm)\n"));
			delete [] lemr;
			success = false;
			break;
		}
		ParseRecord(lemr);
		delete [] lemr;
	}

	delete container;

	DBGMSG(("< EMFRStream\n"));
	return success;
}

pattern EMFTranslator::SelectStrokePen()
{
	rgb_color color;

	view->SetPenSize(max(dc.lp.lopnWidth.x, dc.lp.lopnWidth.y));
	color.red   = GetRValue(dc.lp.lopnColor);
	color.green = GetGValue(dc.lp.lopnColor);
	color.blue  = GetBValue(dc.lp.lopnColor);
	color.alpha = 255;
	view->SetHighColor(color);
	return B_SOLID_HIGH;
}

pattern EMFTranslator::SelectFillBrush()
{
	pattern high_color;
	rgb_color color;

	switch (dc.lb.lbStyle) {
	case BS_SOLID:
		high_color = B_SOLID_HIGH;
		color.red   = GetRValue(dc.lb.lbColor);
		color.green = GetGValue(dc.lb.lbColor);
		color.blue  = GetBValue(dc.lb.lbColor);
		color.alpha = 255;
		view->SetHighColor(color);
		break;

	case BS_NULL:
		high_color = B_SOLID_HIGH;
		break;

	case BS_HATCHED:
		switch (dc.lb.lbHatch) {
		case HS_HORIZONTAL:
			high_color = HORIZONTAL;
			break;
		case HS_VERTICAL:
			high_color = VERTICAL;
			break;
		case HS_FDIAGONAL:
			high_color = FDIAGONAL;
			break;
		case HS_BDIAGONAL:
			high_color = BDIAGONAL;
			break;
		case HS_CROSS:
			high_color = CROSS;
			break;
		case HS_DIAGCROSS:
			high_color = DIAGCROSS;
			break;
		}
		color.red   = GetRValue(dc.lb.lbColor);
		color.green = GetGValue(dc.lb.lbColor);
		color.blue  = GetBValue(dc.lb.lbColor);
		color.alpha = 255;
		view->SetHighColor(color);
		break;

	case BS_DIBPATTERN:
	case BS_DIBPATTERNPT:
	case BS_DIBPATTERN8X8:
		high_color = dc.brush_pat;
		break;

	case BS_PATTERN8X8:
	case BS_PATTERN:
	case BS_MONOPATTERN:
		high_color = dc.brush_pat;
		color.red   = GetRValue(dc.crTextColor);
		color.green = GetGValue(dc.crTextColor);
		color.blue  = GetBValue(dc.crTextColor);
		color.alpha = 255;
		view->SetHighColor(color);
		break;
/*
	case BS_INDEXED:
		brush = -1;
		break;
*/
	}
	return high_color;
}

void EMFTranslator::SelectTextFont()
{
	BFont font;
	font.SetSize(abs(dc.lf.lfHeight));
	font.SetRotation(dc.lf.lfEscapement / 10.0f);
	if (dc.lf.lfPitchAndFamily & VARIABLE_PITCH) {
		font.SetSpacing(B_CHAR_SPACING);
	} else {
		font.SetSpacing(B_FIXED_SPACING);
	}
	uint16 face = 0;
	if (dc.lf.lfItalic) {
		face |= B_ITALIC_FACE;
	}
	if (dc.lf.lfUnderline) {
		face |= B_UNDERSCORE_FACE;
	}
	if (dc.lf.lfStrikeOut) {
		face |= B_STRIKEOUT_FACE;
	}
	if (dc.lf.lfWeight >= FW_BOLD) {
		face |= B_BOLD_FACE;
	}
	font.SetFace(face);
	view->SetFont(&font);
}

void EMFTranslator::DefaultHandler(ENHMETARECORD *emr)
{
	switch (emr->iType) {
	case EMR_HEADER:
		DBGMSG(("EMR_HEADER\n"));
		break;
	case EMR_EOF:
		DBGMSG(("EMR_EOF\n"));
		break;
	default:
		DBGMSG(("emr->iType = %d\n", emr->iType));
		break;
	}
}

void EMFTranslator::PolyLineTo(EMRPOLYLINETO *p)
{
	DBGMSG(("PolyLineTo\n"));

	if (p->cptl > 1 && is_valid_stroke_pen()) {
		POINTL *ptl = p->aptl;
		BPoint *pointList = new BPoint[p->cptl];
		BPoint *pl = pointList;
		for (uint32 i = 0 ; i < p->cptl ; i++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", i, ptl->x, ptl->y));
			pl->x = ptl->x;
			pl->y = ptl->y;
			pl++;
			ptl++;
		}
		StrokePolygon(view, pointList, p->cptl, false, SelectStrokePen());
		pl--;
		view->MovePenTo(*pl);
		delete [] pointList;
	}
}

void EMFTranslator::PolyPolyline(EMRPOLYPOLYLINE *p)
{
	DBGMSG(("PolyPolyline\n"));

	if (p->cptl > 1 && is_valid_stroke_pen()) {
		uint32 *polyCounts = p->aPolyCounts;
		POINTL *ptl = p->aptl;
		for (uint32 i = 0 ; i < p->nPolys ; i++) {
			DBGMSG(("\taPolyCounts[%d] = %d\n", i, *polyCounts));
			if (*polyCounts > 1) {
				BPoint *pointList = new BPoint[*polyCounts];
				BPoint *pl = pointList;
				for (uint32 j = 0 ; j < *polyCounts ; j++) {
					DBGMSG(("\taptl[%d] = %d, %d\n", i, ptl->x, ptl->y));
					pl->x = ptl->x;
					pl->y = ptl->y;
					pl++;
					ptl++;
				}
				StrokePolygon(view, pointList, *polyCounts, false, SelectStrokePen());
				delete [] pointList;
			}
			polyCounts++;
		}
	}
}

void EMFTranslator::PolyPolygon(EMRPOLYPOLYGON *p)
{
	DBGMSG(("PolyPolygon\n"));

	if (p->cptl > 1) {
		uint32 *polyCounts = p->aPolyCounts;
		POINTL *ptl = p->aptl;
		for (uint32 i = 0 ; i < p->nPolys ; i++) {
			DBGMSG(("\taPolyCounts[%d] = %d\n", i, *polyCounts));
			if (*polyCounts > 1) {
				BPoint *pointList = new BPoint[*polyCounts];
				BPoint *pl = pointList;
				for (uint32 j = 0 ; j < *polyCounts ; j++) {
					DBGMSG(("\taptl[%d] = %d, %d\n", i, ptl->x, ptl->y));
					pl->x = ptl->x;
					pl->y = ptl->y;
					pl++;
					ptl++;
				}
				if (is_valid_fill_brush()) {
					view->FillPolygon(pointList, *polyCounts, SelectFillBrush());
				}
				delete [] pointList;
			}
			polyCounts++;
		}
		if (is_valid_stroke_pen()) {
			PolyPolyline(p);
		}
	}
}

void EMFTranslator::SetWindowExtEx(EMRSETWINDOWEXTEX *p)
{
	DBGMSG(("SetWindowExtEx\n"));
	DBGMSG(("\tszlExtent = %d, %d\n", p->szlExtent.cx, p->szlExtent.cy));

//	windowExt.x = p->szlExtent.cx;
//	windowExt.y = p->szlExtent.cy;
	if (dc.map_mode == MM_TEXT) {
//		grad = min(viewportExt.x / p->szlExtent.cx, viewportExt.y / p->szlExtent.cy);
		view->SetScale(grad);
		view->SetOrigin(BPoint(windowOrg.x, windowOrg.y));
	} else {
		view->SetScale(2.0);
		view->SetOrigin(BPoint(0.0, 0.0));
	}
}

void EMFTranslator::SetWindowOrgEx(EMRSETWINDOWORGEX *p)
{
	DBGMSG(("SetWindowOrgEx\n"));
	DBGMSG(("\tptlOrigin = %d, %d\n", p->ptlOrigin.x, p->ptlOrigin.y));

	if (dc.map_mode == MM_TEXT) {
		windowOrg.x = 0 - p->ptlOrigin.x;
		windowOrg.y = 0 - p->ptlOrigin.y;
		view->SetScale(grad);
		view->SetOrigin(BPoint(windowOrg.x , windowOrg.y));
	} else {
		view->SetScale(2.0);
		view->SetOrigin(BPoint(0.0, 0.0));
	}
}

void EMFTranslator::SetViewportExtEx(EMRSETVIEWPORTEXTEX *p)
{
	DBGMSG(("SetViewportExtEx\n"));
	DBGMSG(("\tszlExtent = %d, %d\n", p->szlExtent.cx, p->szlExtent.cy));
//	viewportExt.x = p->szlExtent.cx;
//	viewportExt.y = p->szlExtent.cy;
}

void EMFTranslator::SetViewportOrgEx(EMRSETVIEWPORTORGEX *p)
{
	DBGMSG(("SetViewportOrgEx\n"));
	DBGMSG(("\tptlOrigin = %d, %d\n", p->ptlOrigin.x, p->ptlOrigin.y));
//	viewportOrg.x = p->ptlOrigin.x;
//	viewportOrg.y = p->ptlOrigin.y;
}

void EMFTranslator::SetBrushOrgEx(EMRSETBRUSHORGEX *p)
{
	DBGMSG(("SetBrushOrgEx\n"));
	DBGMSG(("\tptlOrigin = %d, %d\n", p->ptlOrigin.x, p->ptlOrigin.y));
}

void EMFTranslator::SetPixelV(EMRSETPIXELV *p)
{
	DBGMSG(("SetPixelV\n"));
}

void EMFTranslator::SetMapperFlags(EMRSETMAPPERFLAGS *p)
{
	DBGMSG(("SetMapperFlags\n"));
}

void EMFTranslator::SetMapMode(EMRSETMAPMODE *p)
{
	DBGMSG(("SetMapMode\n"));
	DBGMSG(("\tiMode = %d\n", p->iMode));

	dc.map_mode = p->iMode;
}

void EMFTranslator::SetBkMode(EMRSETBKMODE *p)
{
	DBGMSG(("SetBkMode\n"));

	switch (p->iMode) {
	case TRANSPARENT:
		DBGMSG(("\tiMode = TRANSPARENT\n"));
		view->SetDrawingMode(B_OP_OVER);
		break;
	case OPAQUE:
		DBGMSG(("\tiMode = OPAQUE\n"));
		view->SetDrawingMode(B_OP_COPY);
		break;
	}
}

void EMFTranslator::SetPolyFillMode(EMRSETPOLYFILLMODE *p)
{
	DBGMSG(("SetPolyFillMode\n"));
}

void EMFTranslator::SetROP2(EMRSETROP2 *p)
{
	DBGMSG(("SetROP2\n"));
	DBGMSG(("\tiMode = %d\n", p->iMode));
}

void EMFTranslator::SetStretchBltMode(EMRSETSTRETCHBLTMODE *p)
{
	DBGMSG(("SetStretchBltMode\n"));
}

void EMFTranslator::SetTextAlign(EMRSETTEXTALIGN *p)
{
	DBGMSG(("SetTextAlign\n"));
	DBGMSG(("\tiMode = %d\n", p->iMode));
}

void EMFTranslator::SetColorAdjustment(EMRSETCOLORADJUSTMENT *p)
{
	DBGMSG(("SetColorAdjustment\n"));
}

void EMFTranslator::SetTextColor(EMRSETTEXTCOLOR *p)
{
	DBGMSG(("SetTextColor\n"));
	DBGMSG(("\tcrColor = 0x%x\n", p->crColor));
	dc.crTextColor = p->crColor;
}

void EMFTranslator::SetBkColor(EMRSETBKCOLOR *p)
{
	DBGMSG(("SetBkColor\n"));
	DBGMSG(("\tcrColor = 0x%x\n", p->crColor));

	rgb_color color;
	color.red   = GetRValue(p->crColor);
	color.green = GetGValue(p->crColor);
	color.blue  = GetBValue(p->crColor);
	color.alpha = 255;
	view->SetLowColor(color);
}

void EMFTranslator::OffsetClipRgn(EMROFFSETCLIPRGN *p)
{
	DBGMSG(("OffsetClipRgn\n"));
}

void EMFTranslator::MoveToEx(EMRMOVETOEX *p)
{
	DBGMSG(("MoveToEx\n"));
	DBGMSG(("\tptl = %d, %d\n", p->ptl.x, p->ptl.y));

	BPoint pt(p->ptl.x, p->ptl.y); 
	view->MovePenTo(pt);
}

void EMFTranslator::SetMetaRgn(EMRSETMETARGN *p)
{
	DBGMSG(("SetMetaRgn\n"));
}

void EMFTranslator::ExcludeClipRect(EMREXCLUDECLIPRECT *p)
{
	DBGMSG(("ExcludeClipRect\n"));
	DBGMSG(("\trclClip = %d, %d, %d, %d\n", p->rclClip.left, p->rclClip.top, p->rclClip.right, p->rclClip.bottom));

	BRect rect(p->rclClip.left, p->rclClip.top, p->rclClip.right, p->rclClip.bottom);
	BRegion region;
	view->GetClippingRegion(&region);
	region.Exclude(rect);
	view->ConstrainClippingRegion(&region);
}

void EMFTranslator::IntersectClipRect(EMRINTERSECTCLIPRECT *p)
{
	DBGMSG(("IntersectClipRect\n"));
	DBGMSG(("\trclClip = %d, %d, %d, %d\n", p->rclClip.left, p->rclClip.top, p->rclClip.right, p->rclClip.bottom));

	BRegion r;
	r.Set(BRect(p->rclClip.left, p->rclClip.top, p->rclClip.right, p->rclClip.bottom));
/*
	BRegion region;
	view->GetClippingRegion(&region);
	region.IntersectWith(&r);

	view->ConstrainClippingRegion(&region);
*/
//	view->ConstrainClippingRegion(&r);
}

void EMFTranslator::ScaleViewportExtEx(EMRSCALEVIEWPORTEXTEX *p)
{
	DBGMSG(("\tScaleViewportExtEx\n"));
	DBGMSG(("\txNum = %d\n", p->xNum));
	DBGMSG(("\txDenom = %d\n", p->xDenom));
	DBGMSG(("\tyNum = %d\n", p->yNum));
	DBGMSG(("\tyDenom = %d\n", p->yDenom));
}

void EMFTranslator::ScaleWindowExtEx(EMRSCALEWINDOWEXTEX *p)
{
	DBGMSG(("ScaleWindowExtEx\n"));
	DBGMSG(("\txNum = %d\n", p->xNum));
	DBGMSG(("\txDenom = %d\n", p->xDenom));
	DBGMSG(("\tyNum = %d\n", p->yNum));
	DBGMSG(("\tyDenom = %d\n", p->yDenom));
}

void EMFTranslator::SaveDC(EMRSAVEDC *p)
{
	DBGMSG(("SaveDC\n"));
}

void EMFTranslator::RestoreDC(EMRRESTOREDC *p)
{
	DBGMSG(("RestoreDC\n"));
}

void EMFTranslator::SetWorldTransform(EMRSETWORLDTRANSFORM *p)
{
	DBGMSG(("SetWorldTransform\n"));
	DUMP_XFORM(&p->xform);
}

void EMFTranslator::ModifyWorldTransform(EMRMODIFYWORLDTRANSFORM *p)
{
	DBGMSG(("ModifyWorldTransform\n"));
	DUMP_XFORM(&p->xform);
	DBGMSG(("\tiMode = %d\n", p->iMode));
}

void EMFTranslator::SelectObject(EMRSELECTOBJECT *p)
{
	DBGMSG(("SelectObject\n"));
	DBGMSG(("\tihObject = %d\n", p->ihObject));

	if (p->ihObject & ENHMETA_STOCK_OBJECT) {
		switch (p->ihObject & ~ENHMETA_STOCK_OBJECT) {
		case WHITE_BRUSH:
			DBGMSG(("\ttock object: WHITE_BRUSH\n"));
			dc.lb.lbStyle = BS_SOLID;
			dc.lb.lbColor = 0xffffff;
			dc.lb.lbHatch = 0;
			break;
		case LTGRAY_BRUSH:
			DBGMSG(("\tstock object: LTGRAY_BRUSH\n"));
			cout << '\t' << "" << '\n';
			dc.lb.lbStyle = BS_SOLID;
			dc.lb.lbColor = 0xc0c0c0;
			dc.lb.lbHatch = 0;
			break;
		case GRAY_BRUSH:
			DBGMSG(("\tstock object: GRAY_BRUSH\n"));
			dc.lb.lbStyle = BS_SOLID;
			dc.lb.lbColor = 0x808080;
			dc.lb.lbHatch = 0;
			break;
		case DKGRAY_BRUSH:
			DBGMSG(("\tstock object: DKGRAY_BRUSH\n"));
			dc.lb.lbStyle = BS_SOLID;
			dc.lb.lbColor = 0x404040;
			dc.lb.lbHatch = 0;
			break;
		case BLACK_BRUSH:
			DBGMSG(("\tstock object: BLACK_BRUSH\n"));
			dc.lb.lbStyle = BS_SOLID;
			dc.lb.lbColor = 0;
			dc.lb.lbHatch = 0;
			break;
		case NULL_BRUSH:
			DBGMSG(("\tstock object: NULL_BRUSH\n"));
			dc.lb.lbStyle = BS_NULL;
			dc.lb.lbColor = 0;
			dc.lb.lbHatch = 0;
			break;
		case WHITE_PEN:
			DBGMSG(("\tstock object: WHITE_PEN\n"));
			dc.lp.lopnStyle   = PS_SOLID;
			dc.lp.lopnWidth.x = 0;
			dc.lp.lopnWidth.y = 0;
			dc.lp.lopnColor   = 0xffffff;
			break;
		case BLACK_PEN:
			DBGMSG(("\tstock object: BLACK_PEN\n"));
			dc.lp.lopnStyle   = PS_SOLID;
			dc.lp.lopnWidth.x = 0;
			dc.lp.lopnWidth.y = 0;
			dc.lp.lopnColor   = 0;
			break;
		case NULL_PEN:
			DBGMSG(("\tstock object: NULL_PEN\n"));
			dc.lp.lopnStyle   = PS_NULL;
			dc.lp.lopnWidth.x = 1;
			dc.lp.lopnWidth.y = 0;
			dc.lp.lopnColor   = 0;
			break;
		case OEM_FIXED_FONT:
			DBGMSG(("\tstock object: OEM_FIXED_FONT\n"));
			dc.lf.lfHeight         = 18;
			dc.lf.lfWidth          = 8;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = SHIFTJIS_CHARSET;
			dc.lf.lfOutPrecision   = OUT_STRING_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_STROKE_PRECIS;
			dc.lf.lfQuality        = PROOF_QUALITY;
			dc.lf.lfPitchAndFamily = DEFAULT_PITCH;
//			strcpy(dc.lf.lfFaceName, "Terminal");
			break;
		case ANSI_FIXED_FONT:
			DBGMSG(("\tstock object: ANSI_FIXED_FONT\n"));
			dc.lf.lfHeight         = 12;
			dc.lf.lfWidth          = 9;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = ANSI_CHARSET;
			dc.lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_STROKE_PRECIS;
			dc.lf.lfQuality        = PROOF_QUALITY;
			dc.lf.lfPitchAndFamily = DEFAULT_PITCH;
//			strcpy(dc.lf.lfFaceName, "Courier");
			break;
		case ANSI_VAR_FONT:
			DBGMSG(("\tstock object: ANSI_VAR_FONT\n"));
			dc.lf.lfHeight         = 12;
			dc.lf.lfWidth          = 9;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = ANSI_CHARSET;
			dc.lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_STROKE_PRECIS;
			dc.lf.lfQuality        = PROOF_QUALITY;
			dc.lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
//			strcpy(dc.lf.lfFaceName, "MS Sans Serif");
			break;
		case SYSTEM_FONT:
			DBGMSG(("\tstock object: SYSTEM_FONT\n"));
			dc.lf.lfHeight         = 18;
			dc.lf.lfWidth          = 8;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = SHIFTJIS_CHARSET;
			dc.lf.lfOutPrecision   = OUT_STRING_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_STROKE_PRECIS;
			dc.lf.lfQuality        = PROOF_QUALITY;
			dc.lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
//			strcpy(dc.lf.lfFaceName, "System");
			break;
		case DEVICE_DEFAULT_FONT:
			DBGMSG(("\tstock object: DEVICE_DEFAULT_FONT\n"));
			dc.lf.lfHeight         = 18;
			dc.lf.lfWidth          = 8;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = SHIFTJIS_CHARSET;
			dc.lf.lfOutPrecision   = OUT_STRING_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_STROKE_PRECIS;
			dc.lf.lfQuality        = PROOF_QUALITY;
			dc.lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
//			strcpy(dc.lf.lfFaceName, "System");
			break;
		case DEFAULT_PALETTE:
			DBGMSG(("\tstock object: DEFAULT_PALETTE\n"));
			break;
		case SYSTEM_FIXED_FONT:
			DBGMSG(("\tstock object: SYSTEM_FIXED_FONT\n"));
			dc.lf.lfHeight			= 18;
			dc.lf.lfWidth			= 8;
			dc.lf.lfEscapement		= 0;
			dc.lf.lfOrientation		= 0;
			dc.lf.lfWeight			= FW_NORMAL;
			dc.lf.lfItalic			= 0;
			dc.lf.lfUnderline		= 0;
			dc.lf.lfStrikeOut		= 0;
			dc.lf.lfCharSet			= SHIFTJIS_CHARSET;
			dc.lf.lfOutPrecision	= OUT_STRING_PRECIS;
			dc.lf.lfClipPrecision	= CLIP_STROKE_PRECIS;
			dc.lf.lfQuality			= PROOF_QUALITY;
			dc.lf.lfPitchAndFamily	= DEFAULT_PITCH;
//			strcpy(dc.lf.lfFaceName, "FixedSys");
			break;
		case DEFAULT_GUI_FONT:
			DBGMSG(("\tstock object: DEFAULT_GUI_FONT\n"));
			dc.lf.lfHeight         = -12;
			dc.lf.lfWidth          = 0;
			dc.lf.lfEscapement     = 0;
			dc.lf.lfOrientation    = 0;
			dc.lf.lfWeight         = FW_NORMAL;
			dc.lf.lfItalic         = 0;
			dc.lf.lfUnderline      = 0;
			dc.lf.lfStrikeOut      = 0;
			dc.lf.lfCharSet        = SHIFTJIS_CHARSET;
			dc.lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
			dc.lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
			dc.lf.lfQuality        = DEFAULT_QUALITY;
			dc.lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
//			strcpy(dc.lf.lfFaceName, "‚l‚r ‚oƒSƒVƒbƒN");
			break;
		case DC_BRUSH:
			DBGMSG(("\tstock object: DC_BRUSH\n"));
			dc.lb.lbStyle = BS_NULL;
//			dc.lb.lbColor = 0xffffff;
			dc.lb.lbHatch = 0;
			break;
		case DC_PEN:
			DBGMSG(("\tstock object: DC_PEN\n"));
			dc.lp.lopnStyle   = PS_SOLID;
			dc.lp.lopnWidth.x = 0;
			dc.lp.lopnWidth.y = 0;
//			dc.lp.lopnColor   = 0;
			break;
		}
	} else {
		GDIObject *obj = container->get_at(p->ihObject);
		switch (obj->id) {
		case OBJ_PEN:
			dc.lp = ((GDIPen *)obj)->lp;
			dc.pen_pat = ((GDIPen *)obj)->pat;
			break;
		case OBJ_BRUSH:
			dc.lb = ((GDIBrush *)obj)->lb;
			dc.brush_pat = ((GDIBrush *)obj)->pat;
			break;
		case OBJ_FONT:
			dc.lf = ((GDIFont *)obj)->lf;
			SelectTextFont();
			break;
		case OBJ_PAL:
			break;
		case OBJ_COLORSPACE:
			break;
		default:
			break;
		}
	}
}

void EMFTranslator::CreatePen(EMRCREATEPEN *p)
{
	DBGMSG(("CreatePen\n"));
	DBGMSG(("\tihPen = %d\n", p->ihPen));
	DUMP_LOGPEN(&p->lopn);

	if (!(p->ihPen & ENHMETA_STOCK_OBJECT)) {
		container->set_at(p->ihPen, new GDIPen(&p->lopn));
	}
}

void EMFTranslator::CreateBrushIndirect(EMRCREATEBRUSHINDIRECT *p)
{
	DBGMSG(("CreateBrushIndirect\n"));
	DBGMSG(("\tihBrush = %d\n", p->ihBrush));
	DUMP_LOGBRUSH(&p->lb);

	if (!(p->ihBrush & ENHMETA_STOCK_OBJECT)) {
		container->set_at(p->ihBrush, new GDIBrush(&p->lb));
	}
}

void EMFTranslator::DeleteObject(EMRDELETEOBJECT *p)
{
	DBGMSG(("DeleteObject\n"));
	DBGMSG(("\tihObject = %d\n", p->ihObject));
}

void EMFTranslator::AngleArc(EMRANGLEARC *p)
{
	DBGMSG(("AngleArc\n"));

	if (is_valid_stroke_pen()) {
		BPoint center(p->ptlCenter.x, p->ptlCenter.y);
		view->StrokeArc(center, p->nRadius, p->nRadius, p->eStartAngle, p->eSweepAngle, SelectStrokePen());
	}
}

void EMFTranslator::Ellipse(EMRELLIPSE *p)
{
	DBGMSG(("Ellipse\n"));
	DBGMSG(("\trclBox = %d, %d, %d, %d\n", p->rclBox.left, p->rclBox.top, p->rclBox.right, p->rclBox.bottom));

	BRect rect(p->rclBox.left, p->rclBox.top, p->rclBox.right, p->rclBox.bottom);
	if (is_valid_fill_brush()) {
		view->FillEllipse(rect, SelectFillBrush());
	}
	if (is_valid_stroke_pen()) {
		view->StrokeEllipse(rect, SelectStrokePen());
	}
}

void EMFTranslator::Rectangle(EMRRECTANGLE *p)
{
	DBGMSG(("Rectangle\n"));
	DBGMSG(("\trclBox = %d, %d, %d, %d\n", p->rclBox.left, p->rclBox.top, p->rclBox.right, p->rclBox.bottom));

	BRect rect(p->rclBox.left, p->rclBox.top, p->rclBox.right, p->rclBox.bottom);
	if (is_valid_fill_brush()) {
		view->FillRect(rect, SelectFillBrush());
	}
	if (is_valid_stroke_pen()) {
		view->StrokeRect(rect, SelectStrokePen());
	}
}

void EMFTranslator::RoundRect(EMRROUNDRECT *p)
{
	DBGMSG(("RoundRect\n"));
	DBGMSG(("\trclBox = %d, %d, %d, %d\n", p->rclBox.left, p->rclBox.top, p->rclBox.right, p->rclBox.bottom));
	DBGMSG(("\tszlCorner = %d, %d\n", p->szlCorner, p->szlCorner.cy));

	BRect rect;
	rect.left   = p->rclBox.left;
	rect.top    = p->rclBox.top;
	rect.right  = p->rclBox.right;
	rect.bottom = p->rclBox.bottom;

	if (is_valid_fill_brush()) {
		view->FillRoundRect(rect, p->szlCorner.cx, p->szlCorner.cy, SelectFillBrush());
	}
	if (is_valid_stroke_pen()) {
		view->StrokeRoundRect(rect, p->szlCorner.cx, p->szlCorner.cy, SelectStrokePen());
	}
}

void EMFTranslator::Arc(EMRARC *p)
{
	DBGMSG(("Arc\n"));
}

void EMFTranslator::Chord(EMRCHORD *p)
{
	DBGMSG(("Chord\n"));
}

void EMFTranslator::Pie(EMRPIE *p)
{
	DBGMSG(("Pie\n"));
}

void EMFTranslator::SelectPalette(EMRSELECTPALETTE *p)
{
	DBGMSG(("SelectPalette\n"));
}

void EMFTranslator::CreatePalette(EMRCREATEPALETTE *p)
{
	DBGMSG(("CreatePalette\n"));
}

void EMFTranslator::SetPaletteEntries(EMRSETPALETTEENTRIES *p)
{
	DBGMSG(("SetPaletteEntries\n"));
}

void EMFTranslator::ResizePalette(EMRRESIZEPALETTE *p)
{
	DBGMSG(("ResizePalette\n"));
}

void EMFTranslator::RealizePalette(EMRREALIZEPALETTE *p)
{
	DBGMSG(("RealizePalette\n"));
}

void EMFTranslator::ExtFloodFill(EMREXTFLOODFILL *p)
{
	DBGMSG(("ExtFloodFill\n"));
}

void EMFTranslator::LineTo(EMRLINETO *p)
{
	DBGMSG(("LineTo\n"));
	DBGMSG(("\tptl = %d, %d\n", p->ptl.x, p->ptl.y));

	if (is_valid_stroke_pen()) {
		BPoint pt(p->ptl.x, p->ptl.y); 
		view->StrokeLine(pt, SelectStrokePen());
	}
}

void EMFTranslator::ArcTo(EMRARCTO *p)
{
	DBGMSG(("ArcTo\n"));
}

void EMFTranslator::PolyDraw(EMRPOLYDRAW *p)
{
	DBGMSG(("PolyDraw\n"));
}

void EMFTranslator::SetArcDirection(EMRSETARCDIRECTION *p)
{
	DBGMSG(("SetArcDirection\n"));
}

void EMFTranslator::SetMiterLimit(EMRSETMITERLIMIT *p)
{
	DBGMSG(("SetMiterLimit\n"));
}

void EMFTranslator::BeginPath(EMRBEGINPATH *p)
{
	DBGMSG(("BeginPath\n"));
}

void EMFTranslator::EndPath(EMRENDPATH *p)
{
	DBGMSG(("EndPath\n"));
}

void EMFTranslator::CloseFigure(EMRCLOSEFIGURE *p)
{
	DBGMSG(("CloseFigure\n"));
}

void EMFTranslator::FillPath(EMRFILLPATH *p)
{
	DBGMSG(("FillPath\n"));
}

void EMFTranslator::StrokeAndFillPath(EMRSTROKEANDFILLPATH *p)
{
	DBGMSG(("StrokeAndFillPath\n"));
}

void EMFTranslator::StrokePath(EMRSTROKEPATH *p)
{
	DBGMSG(("StrokePath\n"));
}

void EMFTranslator::FlattenPath(EMRFLATTENPATH *p)
{
	DBGMSG(("FlattenPath\n"));
}

void EMFTranslator::WidenPath(EMRWIDENPATH *p)
{
	DBGMSG(("WidenPath\n"));
}

void EMFTranslator::SelectClipPath(EMRSELECTCLIPPATH *p)
{
	DBGMSG(("SelectClipPath\n"));
}

void EMFTranslator::AbortPath(EMRABORTPATH *p)
{
	DBGMSG(("AbortPath\n"));
}

void EMFTranslator::GdiComment(EMRGDICOMMENT *p)
{
	DBGMSG(("GdiComment\n"));
}

void EMFTranslator::FillRgn(EMRFILLRGN *p)
{
	DBGMSG(("FillRgn\n"));
}

void EMFTranslator::FrameRgn(EMRFRAMERGN *p)
{
	DBGMSG(("FrameRgn\n"));
}

void EMFTranslator::InvertRgn(EMRINVERTRGN *p)
{
	DBGMSG(("InvertRgn\n"));
}

void EMFTranslator::PaintRgn(EMRPAINTRGN *p)
{
	DBGMSG(("PaintRgn\n"));
}

void EMFTranslator::ExtSelectClipRgn(EMREXTSELECTCLIPRGN *p)
{
	DBGMSG(("ExtSelectClipRgn\n"));
}

void EMFTranslator::BitBlt(EMRBITBLT *p)
{
	DBGMSG(("BitBlt\n"));
}

void EMFTranslator::StretchBlt(EMRSTRETCHBLT *p)
{
	DBGMSG(("StretchBlt\n"));
}

void EMFTranslator::MaskBlt(EMRMASKBLT *p)
{
	DBGMSG(("MaskBlt\n"));
}

void EMFTranslator::PlgBlt(EMRPLGBLT *p)
{
	DBGMSG(("PlgBlt\n"));
}

void EMFTranslator::SetDIBitsToDevice(EMRSETDIBITSTODEVICE *p)
{
	DBGMSG(("SetDIBitsToDevice\n"));
}

void EMFTranslator::StretchDIBits(EMRSTRETCHDIBITS *p)
{
	DBGMSG(("StretchDIBits\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\txDest     = %d\n", p->xDest));
	DBGMSG(("\tyDest     = %d\n", p->yDest));
	DBGMSG(("\txSrc      = %d\n", p->xSrc));
	DBGMSG(("\tySrc      = %d\n", p->ySrc));
	DBGMSG(("\tcxSrc     = %d\n", p->cxSrc));
	DBGMSG(("\tcySrc     = %d\n", p->cySrc));
	DBGMSG(("\toffBmiSrc = %d\n", p->offBmiSrc));
	DBGMSG(("\tcbBmiSrc  = %d\n", p->cbBmiSrc));
	DBGMSG(("\toffBitsSrc= %d\n", p->offBitsSrc));
	DBGMSG(("\tcbBitsSrc = %d\n", p->cbBitsSrc));
	DBGMSG(("\tiUsageSrc = %d\n", p->iUsageSrc));
	DBGMSG(("\tdwRop     = %d\n", p->dwRop));
	DBGMSG(("\tcxDest    = %d\n", p->cxDest));
	DBGMSG(("\tcyDest    = %d\n", p->cyDest));

	BITMAPINFOHEADER *pbih = (BITMAPINFOHEADER *)((uint8 *)p + p->offBmiSrc);
	rgb_color *palette = (rgb_color *)((uchar *)pbih + pbih->biSize);

	DUMP_BITMAPINFOHEADER(pbih);

	const rgb_color white = { 0xff, 0xff, 0xff, 0xff };
	int delta = ((p->cxSrc + 3) / 4) * 4;
	int rest  = delta - p->cxSrc;

	BRect rcSrc(0.0f, 0.0f, delta - 1, p->cySrc - 1);
	BBitmap *bitmap = new BBitmap(rcSrc, B_RGB32);
	rgb_color *dst = (rgb_color *)bitmap->Bits();

	switch (pbih->biBitCount) {
	case 8:
		{
			uchar *src = (uchar *)p + p->offBitsSrc + p->cxSrc * (p->cySrc - 1);
			uchar *src0;
			for (int i = 0; i < p->cySrc; i++) {
				src0 = src;
				for (int j = 0; j < p->cxSrc; j++) {
					*dst++ = palette[*src++];
				}
				for (int k = 0; k < rest ; k++) {
					*dst++ = white;
				}
				src = src0 - delta;
			}
		}
		break;
	}

	BRect rcDst(p->xDest, p->yDest, p->xDest + p->cxDest - 1, p->yDest + p->cyDest - 1);
	view->DrawBitmap(bitmap, rcDst);
	delete bitmap;
}

void EMFTranslator::ExtCreateFontIndirectW(EMREXTCREATEFONTINDIRECTW *p)
{
	DBGMSG(("ExtCreateFontIndirectW\n"));
	DBGMSG(("\tihFont = %d\n", p->ihFont));
	DUMP_EXTLOGFONT(&p->elfw);

	if (!(p->ihFont & ENHMETA_STOCK_OBJECT)) {
		container->set_at(p->ihFont, new GDIFont(&p->elfw.elfLogFont));
	}
}

void EMFTranslator::ExtTextOutA(EMREXTTEXTOUTA *p)
{
	DBGMSG(("ExtTextOutA\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tiGraphicsMode = %d\n", p->iGraphicsMode));
	DBGMSG(("\texScale = %f\n", p->exScale));
	DBGMSG(("\teyScale = %f\n", p->eyScale));
	DUMP_EMRTEXT(&p->emrtext);

	char *pszSrc = (char *)p + p->emrtext.offString;
	int32 lenSrc = p->emrtext.nChars;
	int32 lenDst = lenSrc * 3 + 1;
	char *pszDst = new char[lenDst];

	int32 state = 0;
	::convert_to_utf8(B_SJIS_CONVERSION, pszSrc, &lenSrc, pszDst, &lenDst, &state);

	BPoint pt(p->emrtext.ptlReference.x, p->emrtext.ptlReference.y);

	pszDst[lenDst] = '\0';
	DBGMSG(("\tstring = %s\n", pszDst));

	rgb_color color;
	color.red   = GetRValue(dc.crTextColor);
	color.green = GetGValue(dc.crTextColor);
	color.blue  = GetBValue(dc.crTextColor);
	color.alpha = 255;
	view->SetHighColor(color);

	view->DrawString(pszDst, lenDst, pt);

	delete [] pszDst;
}

void EMFTranslator::ExtTextOutW(EMREXTTEXTOUTW *p)
{
	DBGMSG(("ExtTextOutW\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tiGraphicsMode = %d\n", p->iGraphicsMode));
	DBGMSG(("\texScale = %f\n", p->exScale));
	DBGMSG(("\teyScale = %f\n", p->eyScale));
	DUMP_EMRTEXT(&p->emrtext);

	char *pszSrc = (char *)p + p->emrtext.offString;
	int32 lenSrc = p->emrtext.nChars;
	int32 lenDst = lenSrc * 3 + 1;
	char *pszDst = new char[lenDst];

//	int32 state = 0;
//	::convert_to_utf8(B_UNICODE_CONVERSION, pszSrc, &lenSrc, pszDst, &lenDst, &state);
	convert_unicode_to_utf8((wchar_t *)pszSrc, lenSrc, pszDst, &lenDst);

	BPoint pt(p->emrtext.ptlReference.x, p->emrtext.ptlReference.y);

	pszDst[lenDst] = '\0';
	DBGMSG(("\tstring = %s\n", pszDst));

	rgb_color color;
	color.red   = GetRValue(dc.crTextColor);
	color.green = GetGValue(dc.crTextColor);
	color.blue  = GetBValue(dc.crTextColor);
	color.alpha = 255;
	view->SetHighColor(color);

	view->DrawString(pszDst, lenDst, pt);

	delete [] pszDst;
}

void EMFTranslator::PolyBezier16(EMRPOLYBEZIER16 *p)
{
	DBGMSG(("PolyBezier16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (is_valid_stroke_pen()) {
		POINTS *pts = p->apts;
		BPoint *pointList = new BPoint[p->cpts];
		BPoint *pl = pointList;
		for (uint32 j = 0 ; j < p->cpts ; j++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
			pl->x = pts->x;
			pl->y = pts->y;
			pl++;
			pts++;
		}
		view->StrokeBezier(pointList, SelectStrokePen());
		delete [] pointList;
	}
}

void EMFTranslator::Polygon16(EMRPOLYGON16 *p)
{
	DBGMSG(("Polygon16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (p->cpts > 1) {
		POINTS *pts = p->apts;
		BPoint *pointList = new BPoint[p->cpts];
		BPoint *pl = pointList;
		for (uint32 j = 0 ; j < p->cpts ; j++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
			pl->x = pts->x;
			pl->y = pts->y;
			pl++;
			pts++;
		}
		if (is_valid_fill_brush()) {
			view->FillPolygon(pointList, p->cpts, SelectFillBrush());
		}
		if (is_valid_stroke_pen()) {
			StrokePolygon(view, pointList, p->cpts, true, SelectStrokePen());
		}
		delete [] pointList;
	}
}

void EMFTranslator::Polyline16(EMRPOLYLINE16 *p)
{
	DBGMSG(("Polyline16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (p->cpts > 1 && is_valid_stroke_pen()) {
		POINTS *pts = p->apts;
		BPoint *pointList = new BPoint[p->cpts];
		BPoint *pl = pointList;
		for (uint32 i = 0 ; i < p->cpts ; i++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", i, (int)pts->x, (int)pts->y));
			pl->x = pts->x;
			pl->y = pts->y;
			pl++;
			pts++;
		}
		StrokePolygon(view, pointList, p->cpts, false, SelectStrokePen());
		delete [] pointList;
	}
}

void EMFTranslator::PolyBezierTo16(EMRPOLYBEZIERTO16 *p)
{
	DBGMSG(("PolyBezierTo16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (p->cpts > 1 && is_valid_stroke_pen()) {
		POINTS *pts = p->apts;
		BPoint *pointList = new BPoint[p->cpts];
		BPoint *pl = pointList;
		for (uint32 j = 0 ; j < p->cpts ; j++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
			pl->x = pts->x;
			pl->y = pts->y;
			pl++;
			pts++;
		}
		view->StrokeBezier(pointList, SelectStrokePen());
		pl--;
		view->MovePenTo(*pl);
		delete [] pointList;
	}
}

void EMFTranslator::PolylineTo16(EMRPOLYLINETO16 *p)
{
	DBGMSG(("PolylineTo16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (p->cpts > 1 && is_valid_stroke_pen()) {
		POINTS *pts = p->apts;
		BPoint *pointList = new BPoint[p->cpts];
		BPoint *pl = pointList;
		for (uint32 j = 0 ; j < p->cpts ; j++) {
			DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
			pl->x = pts->x;
			pl->y = pts->y;
			pl++;
			pts++;
		}
		StrokePolygon(view, pointList, p->cpts, false, SelectStrokePen());
		pl--;
		view->MovePenTo(*pl);
		delete [] pointList;
	}
}

void EMFTranslator::PolyPolyline16(EMRPOLYPOLYLINE16 *p)
{
	DBGMSG(("PolyPolyline16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tnPolys = %d\n", p->nPolys));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	BPoint org = view->PenLocation();

	if (p->cpts > 1 && is_valid_stroke_pen()) {
		uint32 *polyCounts = p->aPolyCounts;
		POINTS *pts = (POINTS *)(p->aPolyCounts + p->nPolys);
		for (uint32 i = 0 ; i < p->nPolys ; i++) {
			DBGMSG(("\taPolyCounts[%d] = %d\n", i, *polyCounts));
			if (*polyCounts > 1) {
				BPoint *pointList = new BPoint[*polyCounts];
				BPoint *pl = pointList;
				for (uint32 j = 0 ; j < *polyCounts ; j++) {
					DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
					pl->x = pts->x;
					pl->y = pts->y;
					pl++;
					pts++;
				}
				StrokePolygon(view, pointList, *polyCounts, false, SelectStrokePen());
				delete [] pointList;
				polyCounts++;
			}
		}
	}

	view->MovePenTo(org);
}

void EMFTranslator::PolyPolygon16(EMRPOLYPOLYGON16 *p)
{
	DBGMSG(("PolyPolygon16\n"));
	DBGMSG(("\trclBounds = %d, %d, %d, %d\n", p->rclBounds.left, p->rclBounds.top, p->rclBounds.right, p->rclBounds.bottom));
	DBGMSG(("\tnPolys = %d\n", p->nPolys));
	DBGMSG(("\tcpts = %d\n", p->cpts));

	if (p->cpts > 1) {
		uint32 *polyCounts = p->aPolyCounts;
		POINTS *pts = (POINTS *)(p->aPolyCounts + p->nPolys);
		for (uint32 i = 0 ; i < p->nPolys ; i++) {
			DBGMSG(("\taPolyCounts[%d] = %d\n", i, *polyCounts));
			if (*polyCounts > 1) {
				BPoint *pointList = new BPoint[*polyCounts];
				BPoint *pl = pointList;
				for (uint32 j = 0 ; j < *polyCounts ; j++) {
					DBGMSG(("\taptl[%d] = %d, %d\n", j, (int)pts->x, (int)pts->y));
					pl->x = pts->x;
					pl->y = pts->y;
					pl++;
					pts++;
				}
				if (is_valid_fill_brush()) {
					view->FillPolygon(pointList, *polyCounts, SelectFillBrush());
				}
				delete [] pointList;
			}
			polyCounts++;
		}
		if (is_valid_stroke_pen()) {
			PolyPolyline16(p);
		}
	}
}

void EMFTranslator::PolyDraw16(EMRPOLYDRAW16 *p)
{
	DBGMSG(("PolyDraw16\n"));
}

void EMFTranslator::CreateMonoBrush(EMRCREATEMONOBRUSH *p)
{
	DBGMSG(("CreateMonoBrush\n"));
	DBGMSG(("\tihBrush = %d\n", p->ihBrush));
	DBGMSG(("\tiUsage  = %d\n", p->iUsage));
	DBGMSG(("\toffBmi  = %d\n", p->offBmi));
	DBGMSG(("\tcbBmi   = %d\n", p->cbBmi));
	DBGMSG(("\toffBits = %d\n", p->offBits));
	DBGMSG(("\tcbBits  = %d\n", p->cbBits));

	BITMAPINFOHEADER *pbih = (BITMAPINFOHEADER *)((uint8 *)p + p->offBmi);
	DUMP_BITMAPINFOHEADER(pbih);
	uint8 *bits = (uint8 *)p + p->offBits;
	int delta = p->cbBits / pbih->biHeight;
	pattern pat;
	for (int i = 0 ; i < 8 ; i++) {
		pat.data[i] = ~(*bits);
		bits += delta;
	}

	if (!(p->ihBrush & ENHMETA_STOCK_OBJECT)) {
		LOGBRUSH lb;
		lb.lbStyle = BS_PATTERN;
		lb.lbColor = 0;
		lb.lbHatch = 0;
		container->set_at(p->ihBrush, new GDIBrush(&lb, &pat));
	}
}

void EMFTranslator::CreateDIBPatternBrushPt(EMRCREATEDIBPATTERNBRUSHPT *p)
{
	DBGMSG(("CreateDIBPatternBrushPt\n"));
	DBGMSG(("\tihBrush = %d\n", p->ihBrush));
	DBGMSG(("\tiUsage  = %d\n", p->iUsage));
	DBGMSG(("\toffBmi  = %d\n", p->offBmi));
	DBGMSG(("\tcbBmi   = %d\n", p->cbBmi));
	DBGMSG(("\toffBits = %d\n", p->offBits));
	DBGMSG(("\tcbBits  = %d\n", p->cbBits));

	BITMAPINFOHEADER *pbih = (BITMAPINFOHEADER *)((uint8 *)p + p->offBmi);
	COLORREF *palette = (COLORREF *)((uchar *)pbih + pbih->biSize);
	uchar *bits = (uchar *)p + p->offBits;

	DUMP_BITMAPINFOHEADER(pbih);

	if (!(p->ihBrush & ENHMETA_STOCK_OBJECT)) {
		LOGBRUSH lb;
		lb.lbStyle = BS_DIBPATTERNPT;
		lb.lbColor = 0;
		lb.lbHatch = 0;
		container->set_at(p->ihBrush, new GDIBrush(&lb, (pattern *)bits));
	}
}

void EMFTranslator::ExtCreatePen(EMREXTCREATEPEN *p)
{
	DBGMSG(("ExtCreatePen\n"));
}

void EMFTranslator::PolyTextOutA(EMRPOLYTEXTOUTA *p)
{
	DBGMSG(("PolyTextOutA\n"));
}

void EMFTranslator::PolyTextOutW(EMRPOLYTEXTOUTW *p)
{
	DBGMSG(("PolyTextOutW\n"));
}
/*
void EMFTranslator::SetICMMode(EMRSETICMMODE *p)
{
	DBGMSG(("SetICMMode\n"));
}

void EMFTranslator::CreateColorSpace(EMRCREATECOLORSPACE *p)
{
	DBGMSG(("CreateColorSpace\n"));
}

void EMFTranslator::SetColorSpace(EMRSETCOLORSPACE *p)
{
	DBGMSG(("SetColorSpace\n"));
}

void EMFTranslator::DeleteColorSpace(EMRDELETECOLORSPACE *p)
{
	DBGMSG(("DeleteColorSpace\n"));
}

void EMFTranslator::GlsRecord(EMRGLSRECORD *p)
{
	DBGMSG(("GlsRecord\n"));
}

void EMFTranslator::GlsBoundedRecord(EMRGLSBOUNDEDRECORD *p)
{
	DBGMSG(("GlsBoundedRecord\n"));
}

void EMFTranslator::PixelFormat(EMRPIXELFORMAT *p)
{
	DBGMSG(("PixelFormat\n"));
}

void EMFTranslator::DrawEscape(EMR_DRAWESCAPE *p)
{
	DBGMSG(("DrawEscape\n"));
}

void EMFTranslator::ExtEscape(EMR_EXTESCAPE *p)
{
	DBGMSG(("ExtEscape\n"));
}

void EMFTranslator::StartDoc(EMR_STARTDOC *p
{
	DBGMSG(("StartDoc\n"));
}

void EMFTranslator::SmallTextOut(EMRSMALLTEXTOUT *p)
{
	DBGMSG(("SmallTextOut\n"));
}

void EMFTranslator::ForceUFIMapping(EMRFORCEUFIMAPPING *p)
{
	DBGMSG(("ForceUFIMapping\n"));
}

void EMFTranslator::NamedEscape(EMRNAMEDESCAPE *p)
{
	DBGMSG(("NamedEscape\n"));
}

void EMFTranslator::ColorCorrectPalette(EMRCOLORCORRECTPALETTE *p)
{
	DBGMSG(("ColorCorrectPalette\n"));
}

void EMFTranslator::SetICMProfileA(EMRSETICMPROFILEA *p)
{
	DBGMSG(("SetICMProfileA\n"));
}

void EMFTranslator::SetICMProfileW(EMRSETICMPROFILEW *p)
{
	DBGMSG(("SetICMProfileW\n"));
}

void EMFTranslator::AlphaBlend(EMRALPHABLEND *p)
{
	DBGMSG(("AlphaBlend\n"));
}

void EMFTranslator::AlphaDIBBlend(EMRALPHADIBBLEND *p)
{
	DBGMSG(("AlphaDIBBlend\n"));
}

void EMFTranslator::TransparentBlt(EMRTRANSPARENTBLT *p)
{
	DBGMSG(("TransparentBlt\n"));
}

void EMFTranslator::TransparentDIBits(EMRTRANSPARENTDIB *p)
{
	DBGMSG(("TransparentDIBits\n"));
}

void EMFTranslator::GradientFill(EMRGRADIENTFILL *p)
{
	DBGMSG(("GradientFill\n"));
}

void EMFTranslator::SetLinkedUFIs(EMRSETLINKEDUFIS *p)
{
	DBGMSG(("SetLinkedUFIs\n"));
}

void EMFTranslator::SetTextJustification(EMRSETTEXTJUSTIFICATION *p)
{
	DBGMSG(("SetTextJustification\n"));
}
*/
void EMFTranslator::ParseRecord(ENHMETARECORD *emr)
{
//	DBGMSG(("ENHMETARECORD::iType = "));

	switch (emr->iType) {
	case EMR_POLYLINETO:
		PolyLineTo((EMRPOLYLINETO *)emr);
		break;
	case EMR_POLYPOLYLINE:
		PolyPolyline((EMRPOLYPOLYLINE *)emr);
		break;
	case EMR_POLYPOLYGON:
		PolyPolygon((EMRPOLYPOLYGON *)emr);
		break;
	case EMR_SETWINDOWEXTEX:
		SetWindowExtEx((EMRSETWINDOWEXTEX *)emr);
		break;
	case EMR_SETWINDOWORGEX:
		SetWindowOrgEx((EMRSETWINDOWORGEX *)emr);
		break;
	case EMR_SETVIEWPORTEXTEX:
		SetViewportExtEx((EMRSETVIEWPORTEXTEX *)emr);
		break;
	case EMR_SETVIEWPORTORGEX:
		SetViewportOrgEx((EMRSETVIEWPORTORGEX *)emr);
		break;
	case EMR_SETBRUSHORGEX:
		SetBrushOrgEx((EMRSETBRUSHORGEX *)emr);
		break;
	case EMR_SETPIXELV:
		SetPixelV((EMRSETPIXELV *)emr);
		break;
	case EMR_SETMAPPERFLAGS:
		SetMapperFlags((EMRSETMAPPERFLAGS *)emr);
		break;
	case EMR_SETMAPMODE:
		SetMapMode((EMRSETMAPMODE *)emr);
		break;
	case EMR_SETBKMODE:
		SetBkMode((EMRSETBKMODE *)emr);
		break;
	case EMR_SETPOLYFILLMODE:
		SetPolyFillMode((EMRSETPOLYFILLMODE *)emr);
		break;
	case EMR_SETROP2:
		SetROP2((EMRSETROP2 *)emr);
		break;
	case EMR_SETSTRETCHBLTMODE:
		SetStretchBltMode((EMRSETSTRETCHBLTMODE *)emr);
		break;
	case EMR_SETTEXTALIGN:
		SetTextAlign((EMRSETTEXTALIGN *)emr);
		break;
	case EMR_SETCOLORADJUSTMENT:
		SetColorAdjustment((EMRSETCOLORADJUSTMENT *)emr);
		break;
	case EMR_SETTEXTCOLOR:
		SetTextColor((EMRSETTEXTCOLOR *)emr);
		break;
	case EMR_SETBKCOLOR:
		SetBkColor((EMRSETBKCOLOR *)emr);
		break;
	case EMR_OFFSETCLIPRGN:
		OffsetClipRgn((EMROFFSETCLIPRGN *)emr);
		break;
	case EMR_MOVETOEX:
		MoveToEx((EMRMOVETOEX *)emr);
		break;
	case EMR_SETMETARGN:
		SetMetaRgn((EMRSETMETARGN *)emr);
		break;
	case EMR_EXCLUDECLIPRECT:
		ExcludeClipRect((EMREXCLUDECLIPRECT *)emr);
		break;
	case EMR_INTERSECTCLIPRECT:
		IntersectClipRect((EMRINTERSECTCLIPRECT *)emr);
		break;
	case EMR_SCALEVIEWPORTEXTEX:
		ScaleViewportExtEx((EMRSCALEVIEWPORTEXTEX *)emr);
		break;
	case EMR_SCALEWINDOWEXTEX:
		ScaleWindowExtEx((EMRSCALEWINDOWEXTEX *)emr);
		break;
	case EMR_SAVEDC:
		SaveDC((EMRSAVEDC *)emr);
		break;
	case EMR_RESTOREDC:
		RestoreDC((EMRRESTOREDC *)emr);
		break;
	case EMR_SETWORLDTRANSFORM:
		SetWorldTransform((EMRSETWORLDTRANSFORM *)emr);
		break;
	case EMR_MODIFYWORLDTRANSFORM:
		ModifyWorldTransform((EMRMODIFYWORLDTRANSFORM *)emr);
		break;
	case EMR_SELECTOBJECT:
		SelectObject((EMRSELECTOBJECT *)emr);
		break;
	case EMR_CREATEPEN:
		CreatePen((EMRCREATEPEN *)emr);
		break;
	case EMR_CREATEBRUSHINDIRECT:
		CreateBrushIndirect((EMRCREATEBRUSHINDIRECT *)emr);
		break;
	case EMR_DELETEOBJECT:
		DeleteObject((EMRDELETEOBJECT *)emr);
		break;
	case EMR_ANGLEARC:
		AngleArc((EMRANGLEARC *)emr);
		break;
	case EMR_ELLIPSE:
		Ellipse((EMRELLIPSE *)emr);
		break;
	case EMR_RECTANGLE:
		Rectangle((EMRRECTANGLE *)emr);
		break;
	case EMR_ROUNDRECT:
		RoundRect((EMRROUNDRECT *)emr);
		break;
	case EMR_ARC:
		Arc((EMRARC *)emr);
		break;
	case EMR_CHORD:
		Chord((EMRCHORD *)emr);
		break;
	case EMR_PIE:
		Pie((EMRPIE *)emr);
		break;
	case EMR_SELECTPALETTE:
		SelectPalette((EMRSELECTPALETTE *)emr);
		break;
	case EMR_CREATEPALETTE:
		CreatePalette((EMRCREATEPALETTE *)emr);
		break;
	case EMR_SETPALETTEENTRIES:
		SetPaletteEntries((EMRSETPALETTEENTRIES *)emr);
		break;
	case EMR_RESIZEPALETTE:
		ResizePalette((EMRRESIZEPALETTE *)emr);
		break;
	case EMR_REALIZEPALETTE:
		RealizePalette((EMRREALIZEPALETTE *)emr);
		break;
	case EMR_EXTFLOODFILL:
		ExtFloodFill((EMREXTFLOODFILL *)emr);
		break;
	case EMR_LINETO:
		LineTo((EMRLINETO *)emr);
		break;
	case EMR_ARCTO:
		ArcTo((EMRARCTO *)emr);
		break;
	case EMR_POLYDRAW:
		PolyDraw((EMRPOLYDRAW *)emr);
		break;
	case EMR_SETARCDIRECTION:
		SetArcDirection((EMRSETARCDIRECTION *)emr);
		break;
	case EMR_SETMITERLIMIT:
		SetMiterLimit((EMRSETMITERLIMIT *)emr);
		break;
	case EMR_BEGINPATH:
		BeginPath((EMRBEGINPATH *)emr);
		break;
	case EMR_ENDPATH:
		EndPath((EMRENDPATH *)emr);
		break;
	case EMR_CLOSEFIGURE:
		CloseFigure((EMRCLOSEFIGURE *)emr);
		break;
	case EMR_FILLPATH:
		FillPath((EMRFILLPATH *)emr);
		break;
	case EMR_STROKEANDFILLPATH:
		StrokeAndFillPath((EMRSTROKEANDFILLPATH *)emr);
		break;
	case EMR_STROKEPATH:
		StrokePath((EMRSTROKEPATH *)emr);
		break;
	case EMR_FLATTENPATH:
		FlattenPath((EMRFLATTENPATH *)emr);
		break;
	case EMR_WIDENPATH:
		WidenPath((EMRWIDENPATH *)emr);
		break;
	case EMR_SELECTCLIPPATH:
		SelectClipPath((EMRSELECTCLIPPATH *)emr);
		break;
	case EMR_ABORTPATH:
		AbortPath((EMRABORTPATH *)emr);
		break;
	case EMR_GDICOMMENT:
		GdiComment((EMRGDICOMMENT *)emr);
		break;
	case EMR_FILLRGN:
		FillRgn((EMRFILLRGN *)emr);
		break;
	case EMR_FRAMERGN:
		FrameRgn((EMRFRAMERGN *)emr);
		break;
	case EMR_INVERTRGN:
		InvertRgn((EMRINVERTRGN *)emr);
		break;
	case EMR_PAINTRGN:
		PaintRgn((EMRPAINTRGN *)emr);
		break;
	case EMR_EXTSELECTCLIPRGN:
		ExtSelectClipRgn((EMREXTSELECTCLIPRGN *)emr);
		break;
	case EMR_BITBLT:
		BitBlt((EMRBITBLT *)emr);
		break;
	case EMR_STRETCHBLT:
		StretchBlt((EMRSTRETCHBLT *)emr);
		break;
	case EMR_MASKBLT:
		MaskBlt((EMRMASKBLT *)emr);
		break;
	case EMR_PLGBLT:
		PlgBlt((EMRPLGBLT *)emr);
		break;
	case EMR_SETDIBITSTODEVICE:
		SetDIBitsToDevice((EMRSETDIBITSTODEVICE *)emr);
		break;
	case EMR_STRETCHDIBITS:
		StretchDIBits((EMRSTRETCHDIBITS *)emr);
		break;
	case EMR_EXTCREATEFONTINDIRECTW:
		ExtCreateFontIndirectW((EMREXTCREATEFONTINDIRECTW *)emr);
		break;
	case EMR_EXTTEXTOUTA:
		ExtTextOutA((EMREXTTEXTOUTA *)emr);
		break;
	case EMR_EXTTEXTOUTW:
		ExtTextOutW((EMREXTTEXTOUTW *)emr);
		break;
	case EMR_POLYBEZIER16:
		PolyBezier16((EMRPOLYBEZIER16 *)emr);
		break;
	case EMR_POLYGON16:
		Polygon16((EMRPOLYGON16 *)emr);
		break;
	case EMR_POLYLINE16:
		Polyline16((EMRPOLYLINE16 *)emr);
		break;
	case EMR_POLYBEZIERTO16:
		PolyBezierTo16((EMRPOLYBEZIERTO16 *)emr);
		break;
	case EMR_POLYLINETO16:
		PolylineTo16((EMRPOLYLINETO16 *)emr);
		break;
	case EMR_POLYPOLYLINE16:
		PolyPolyline16((EMRPOLYPOLYLINE16 *)emr);
		break;
	case EMR_POLYPOLYGON16:
		PolyPolygon16((EMRPOLYPOLYGON16 *)emr);
		break;
	case EMR_POLYDRAW16:
		PolyDraw16((EMRPOLYDRAW16 *)emr);
		break;
	case EMR_CREATEMONOBRUSH:
		CreateMonoBrush((EMRCREATEMONOBRUSH *)emr);
		break;
	case EMR_CREATEDIBPATTERNBRUSHPT:
		CreateDIBPatternBrushPt((EMRCREATEDIBPATTERNBRUSHPT *)emr);
		break;
	case EMR_EXTCREATEPEN:
		ExtCreatePen((EMREXTCREATEPEN *)emr);
		break;
	case EMR_POLYTEXTOUTA:
		PolyTextOutA((EMRPOLYTEXTOUTA *)emr);
		break;
	case EMR_POLYTEXTOUTW:
		PolyTextOutW((EMRPOLYTEXTOUTW *)emr);
		break;
/*
	case EMR_SETICMMODE:
		SetICMMode((EMRSETICMMODE *)emr);
		break;
	case EMR_CREATECOLORSPACE:
		CreateColorSpace((EMRCREATECOLORSPACE *)emr);
		break;
	case EMR_SETCOLORSPACE:
		SetColorSpace((EMRSETCOLORSPACE *)emr);
		break;
	case EMR_DELETECOLORSPACE:
		DeleteColorSpace((EMRDELETECOLORSPACE *)emr);
		break;
	case EMR_GLSRECORD:
		GlsRecord((EMRGLSRECORD *)emr);
		break;
	case EMR_GLSBOUNDEDRECORD:
		GlsBoundedRecord((EMRGLSBOUNDEDRECORD *)emr);
		break;
	case EMR_PIXELFORMAT:
		PixelFormat((EMRPIXELFORMAT *)emr);
		break;
	case EMR_DRAWESCAPE:
		DrawEscape((EMR_DRAWESCAPE *)emr);
		break;
	case EMR_EXTESCAPE:
		ExtEscape((EMR_EXTESCAPE *)emr);
		break;
	case EMR_STARTDOC:
		StartDoc((EMR_STARTDOC *)emr);
		break;
	case EMR_SMALLTEXTOUT:
		SmallTextOut((EMRSMALLTEXTOUT *)emr);
		break;
	case EMR_FORCEUFIMAPPING:
		ForceUFIMapping((EMRFORCEUFIMAPPING *)emr);
		break;
	case EMR_NAMEDESCAPE:
		NamedEscape((EMRNAMEDESCAPE *)emr);
		break;
	case EMR_COLORCORRECTPALETTE:
		ColorCorrectPalette((EMRCOLORCORRECTPALETTE *)emr);
		break;
	case EMR_SETICMPROFILEA:
		SetICMProfileA((EMRSETICMPROFILE *)emr);
		break;
	case EMR_SETICMPROFILEW:
		SetICMProfileW((EMRSETICMPROFILE *)emr);
		break;
	case EMR_ALPHABLEND:
		AlphaBlend((EMRALPHABLEND *)emr);
		break;
	case EMR_ALPHADIBBLEND:
		AlphaDIBBlend((EMRALPHADIBBLEND *)emr);
		break;
	case EMR_TRANSPARENTBLT:
		TransparentBlt((EMRTRANSPARENTBLT *)emr);
		break;
	case EMR_TRANSPARENTDIB:
		TransparentDIBits((EMRTRANSPARENTDIB *)emr);
		break;
	case EMR_GRADIENTFILL:
		GradientFill((EMRGRADIENTFILL *)emr);
		break;
	case EMR_SETLINKEDUFIS:
		SetLinkedUFIs((EMRSETLINKEDUFIS *)emr);
		break;
	case EMR_SETTEXTJUSTIFICATION:
		SetTextJustification((EMRSETTEXTJUSTIFICATION *)emr);
		break;
*/
	default:
		DefaultHandler(emr);
		break;
	}

	DBGMSG(("\tENHMETARECORD::nSize = %d\n", emr->nSize));
}

/*
int main(int argc, char **argv)
{
	if (argc < 2) {
		DBGMSG(("invalid argument\n"));
		return -1;
	}

	BFile *ifs = new BFile(*++argv, B_READ_ONLY);
	BFile *ofs = new BFile("hogehoge.picture", B_WRITE_ONLY | B_CREATE_FILE);
	EMFTranslator emf;
	emf.ConvertToPicture(ifs, ofs, NULL);
	delete ofs;
	delete ifs;

	ifs = new BFile("hogehoge.picture", B_READ_ONLY);
	BPicture *picture = new BPicture;
	picture->Unflatten(ifs);

	BRect rc(0.0f, 0.0f, 799.0f, 599.0f);
	rc.OffsetTo(30.0, 30.0);
	PreviewWindow *preview = new PreviewWindow(rc, "Preview", picture);
	preview->Go();
	delete picture;
	delete ifs;

	cout << endl;

	return 0;
}
*/