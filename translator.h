/*
 * trnslator.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#ifndef __TRANSLATOR_H
#define __TRANSLATOR_H

#include "gdi.h"

class BMessage;
class BPositionIO;
class GDIContainer;

class DC {
public:
	DC();
	LOGPEN lp;
	LOGBRUSH lb;
	LOGFONTW lf;
	pattern pen_pat;
	pattern brush_pat;
	COLORREF crTextColor;
	int map_mode;
};

class EMFTranslator {
public:
	EMFTranslator() {}
	~EMFTranslator() {}
	status_t ConvertToPicture(BPositionIO *, BPositionIO *, BMessage *);
	status_t ConvertToBitmap(BPositionIO *, BPositionIO *, BMessage *);

protected:
	bool EMFHStream(BPositionIO *, ENHMETAHEADER *);
	bool EMFRStream(BPositionIO *, uint, uint);
	void ParseRecord(ENHMETARECORD *);
	void DefaultHandler(ENHMETARECORD *emr);
	void PolyLineTo(EMRPOLYLINETO *);
	void PolyPolyline(EMRPOLYPOLYLINE *);
	void PolyPolygon(EMRPOLYPOLYGON *);
	void SetWindowExtEx(EMRSETWINDOWEXTEX *);
	void SetWindowOrgEx(EMRSETWINDOWORGEX *);
	void SetViewportExtEx(EMRSETVIEWPORTEXTEX *);
	void SetViewportOrgEx(EMRSETVIEWPORTORGEX *);
	void SetBrushOrgEx(EMRSETBRUSHORGEX *);
	void SetPixelV(EMRSETPIXELV *);
	void SetMapperFlags(EMRSETMAPPERFLAGS *);
	void SetMapMode(EMRSETMAPMODE *);
	void SetBkMode(EMRSETBKMODE *);
	void SetPolyFillMode(EMRSETPOLYFILLMODE *);
	void SetROP2(EMRSETROP2 *);
	void SetStretchBltMode(EMRSETSTRETCHBLTMODE *);
	void SetTextAlign(EMRSETTEXTALIGN *);
	void SetColorAdjustment(EMRSETCOLORADJUSTMENT *);
	void SetTextColor(EMRSETTEXTCOLOR *);
	void SetBkColor(EMRSETBKCOLOR *);
	void OffsetClipRgn(EMROFFSETCLIPRGN *);
	void MoveToEx(EMRMOVETOEX *);
	void SetMetaRgn(EMRSETMETARGN *);
	void ExcludeClipRect(EMREXCLUDECLIPRECT *);
	void IntersectClipRect(EMRINTERSECTCLIPRECT *);
	void ScaleViewportExtEx(EMRSCALEVIEWPORTEXTEX *);
	void ScaleWindowExtEx(EMRSCALEWINDOWEXTEX *);
	void SaveDC(EMRSAVEDC *);
	void RestoreDC(EMRRESTOREDC *);
	void SetWorldTransform(EMRSETWORLDTRANSFORM *);
	void ModifyWorldTransform(EMRMODIFYWORLDTRANSFORM *);
	void SelectObject(EMRSELECTOBJECT *);
	void CreatePen(EMRCREATEPEN *);
	void CreateBrushIndirect(EMRCREATEBRUSHINDIRECT *);
	void DeleteObject(EMRDELETEOBJECT *);
	void AngleArc(EMRANGLEARC *);
	void Ellipse(EMRELLIPSE *);
	void Rectangle(EMRRECTANGLE *);
	void RoundRect(EMRROUNDRECT *);
	void Arc(EMRARC *);
	void Chord(EMRCHORD *);
	void Pie(EMRPIE *);
	void SelectPalette(EMRSELECTPALETTE *);
	void CreatePalette(EMRCREATEPALETTE *);
	void SetPaletteEntries(EMRSETPALETTEENTRIES *);
	void ResizePalette(EMRRESIZEPALETTE *);
	void RealizePalette(EMRREALIZEPALETTE *);
	void ExtFloodFill(EMREXTFLOODFILL *);
	void LineTo(EMRLINETO *);
	void ArcTo(EMRARCTO *);
	void PolyDraw(EMRPOLYDRAW *);
	void SetArcDirection(EMRSETARCDIRECTION *);
	void SetMiterLimit(EMRSETMITERLIMIT *);
	void BeginPath(EMRBEGINPATH *);
	void EndPath(EMRENDPATH *);
	void CloseFigure(EMRCLOSEFIGURE *);
	void FillPath(EMRFILLPATH *);
	void StrokeAndFillPath(EMRSTROKEANDFILLPATH *);
	void StrokePath(EMRSTROKEPATH *);
	void FlattenPath(EMRFLATTENPATH *);
	void WidenPath(EMRWIDENPATH *);
	void SelectClipPath(EMRSELECTCLIPPATH *);
	void AbortPath(EMRABORTPATH *);
	void GdiComment(EMRGDICOMMENT *);
	void FillRgn(EMRFILLRGN *);
	void FrameRgn(EMRFRAMERGN *);
	void InvertRgn(EMRINVERTRGN *);
	void PaintRgn(EMRPAINTRGN *);
	void ExtSelectClipRgn(EMREXTSELECTCLIPRGN *);
	void BitBlt(EMRBITBLT *);
	void StretchBlt(EMRSTRETCHBLT *);
	void MaskBlt(EMRMASKBLT *);
	void PlgBlt(EMRPLGBLT *);
	void SetDIBitsToDevice(EMRSETDIBITSTODEVICE *);
	void StretchDIBits(EMRSTRETCHDIBITS *);
	void ExtCreateFontIndirectW(EMREXTCREATEFONTINDIRECTW *);
	void ExtTextOutA(EMREXTTEXTOUTA *);
	void ExtTextOutW(EMREXTTEXTOUTW *);
	void PolyBezier16(EMRPOLYBEZIER16 *);
	void Polygon16(EMRPOLYGON16 *);
	void Polyline16(EMRPOLYLINE16 *);
	void PolyBezierTo16(EMRPOLYBEZIERTO16 *);
	void PolylineTo16(EMRPOLYLINETO16 *);
	void PolyPolyline16(EMRPOLYPOLYLINE16 *);
	void PolyPolygon16(EMRPOLYPOLYGON16 *);
	void PolyDraw16(EMRPOLYDRAW16 *);
	void CreateMonoBrush(EMRCREATEMONOBRUSH *);
	void CreateDIBPatternBrushPt(EMRCREATEDIBPATTERNBRUSHPT *);
	void ExtCreatePen(EMREXTCREATEPEN *);
	void PolyTextOutA(EMRPOLYTEXTOUTA *);
	void PolyTextOutW(EMRPOLYTEXTOUTW *);
/*
	void SetICMMode(EMRSETICMMODE *p);
	void CreateColorSpace(EMRCREATECOLORSPACE *p);
	void SetColorSpace(EMRSETCOLORSPACE *p);
	void DeleteColorSpace(EMRDELETECOLORSPACE *p);
	void GlsRecord(EMRGLSRECORD *p);
	void GlsBoundedRecord(EMRGLSBOUNDEDRECORD *p);
	void PixelFormat(EMRPIXELFORMAT *p);
	void DrawEscape(EMRDRAWESCAPE *p);
	void ExtEscape(EMREXTESCAPE *p);
	void StartDoc(EMRSTARTDOC *p);
	void SmallTextOut(EMRSMALLTEXTOUT *p);
	void ForceUFIMapping(EMRFORCEUFIMAPPING *p);
	void NamedEscape(EMRNAMEDESCAPE *p);
	void ColorCorrectPalette(EMRCOLORCORRECTPALETTE *p);
	void SetICMProfileA(EMRSETICMPROFILE *p);
	void SetICMProfileW(EMRSETICMPROFILE *p);
	void AlphaBlend(EMRALPHABLEND *p);
	void AlphaDIBBlend(EMRALPHADIBBLEND *p);
	void TransparentBlt(EMRTRANSPARENTBLT *p);
	void TransparentDIBits(EMRTRANSPARENTDIB *p);
	void GradientFill(EMRGRADIENTFILL *p);
	void SetLinkedUFIs(EMRSETLINKEDUFIS *p);
	void SetTextJustification(EMRSETTEXTJUSTIFICATION *p);
*/
	pattern SelectStrokePen();
	pattern SelectFillBrush();
	void SelectTextFont();
	bool is_valid_stroke_pen();
	bool is_valid_fill_brush();

private:
	ENHMETAHEADER emh;
	BView *view;
//	POINTL windowExt;
	POINTL windowOrg;
	BPoint viewportExt;
//	BPoint viewportOrg;
	float grad;
	GDIContainer *container;
	uint nMaxHandles;
	DC dc;
};

inline bool EMFTranslator::is_valid_fill_brush()
{
	if (dc.lb.lbStyle != BS_NULL)
		return true;
	else
		return false;
}

inline bool EMFTranslator::is_valid_stroke_pen()
{
	if (dc.lp.lopnStyle != PS_NULL)
		return true;
	else
		return false;
}

#endif	// __TRANSLATOR_H
