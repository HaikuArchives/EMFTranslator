/*
 * gdi.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#ifndef __GDI_H
#define __GDI_H

#include <SupportDefs.h>

#define MAX_PATH	260

// Object Definitions

#define OBJ_PEN             1
#define OBJ_BRUSH           2
#define OBJ_DC              3
#define OBJ_METADC          4
#define OBJ_PAL             5
#define OBJ_FONT            6
#define OBJ_BITMAP          7
#define OBJ_REGION          8
#define OBJ_METAFILE        9
#define OBJ_MEMDC           10
#define OBJ_EXTPEN          11
#define OBJ_ENHMETADC       12
#define OBJ_ENHMETAFILE     13
#define OBJ_COLORSPACE      14

// ROP2 codes

#define R2_BLACK		1	// 0
#define R2_NOTMERGEPEN	2	// DPon
#define R2_MASKNOTPEN	3	// DPna
#define R2_NOTCOPYPEN	4	// PN
#define R2_MASKPENNOT	5	// PDna
#define R2_NOT			6	// Dn
#define R2_XORPEN		7	// DPx
#define R2_NOTMASKPEN	8	// DPan
#define R2_MASKPEN		9	// DPa
#define R2_NOTXORPEN	10	// DPxn
#define R2_NOP			11	// D
#define R2_MERGENOTPEN	12	// DPno
#define R2_COPYPEN		13	// P
#define R2_MERGEPENNOT	14	// PDno
#define R2_MERGEPEN 	15	// DPo
#define R2_WHITE		16	//	1
#define R2_LAST 		16

// ROP3 codes

#define SRCCOPY 	(DWORD)0x00CC0020 // src
#define SRCPAINT	(DWORD)0x00EE0086 // src OR dest
#define SRCAND		(DWORD)0x008800C6 // src AND dest
#define SRCINVERT	(DWORD)0x00660046 // src XOR dest
#define SRCERASE	(DWORD)0x00440328 // src AND (NOT dest)
#define NOTSRCCOPY	(DWORD)0x00330008 // (NOT src)
#define NOTSRCERASE (DWORD)0x001100A6 // (NOT src) AND (NOT dest)
#define MERGECOPY	(DWORD)0x00C000CA // (src AND pattern)
#define MERGEPAINT	(DWORD)0x00BB0226 // (NOT src) OR dest
#define PATCOPY 	(DWORD)0x00F00021 // pattern
#define PATPAINT	(DWORD)0x00FB0A09 // DPSnoo
#define PATINVERT	(DWORD)0x005A0049 // pattern XOR dest
#define DSTINVERT	(DWORD)0x00550009 // (NOT dest)
#define BLACKNESS	(DWORD)0x00000042 // BLACK
#define WHITENESS	(DWORD)0x00FF0062 // WHITE

#define OUT_DEFAULT_PRECIS			0
#define OUT_STRING_PRECIS			1
#define OUT_CHARACTER_PRECIS		2
#define OUT_STROKE_PRECIS			3
#define OUT_TT_PRECIS				4
#define OUT_DEVICE_PRECIS			5
#define OUT_RASTER_PRECIS			6
#define OUT_TT_ONLY_PRECIS			7
#define OUT_OUTLINE_PRECIS			8
#define OUT_SCREEN_OUTLINE_PRECIS	9

#define CLIP_DEFAULT_PRECIS		0
#define CLIP_CHARACTER_PRECIS	1
#define CLIP_STROKE_PRECIS		2
#define CLIP_MASK				0xf
#define CLIP_LH_ANGLES			(1 << 4)
#define CLIP_TT_ALWAYS			(2 << 4)
#define CLIP_EMBEDDED			(8 << 4)

#define DEFAULT_QUALITY			0
#define DRAFT_QUALITY			1
#define PROOF_QUALITY			2
#define NONANTIALIASED_QUALITY	3
#define ANTIALIASED_QUALITY		4

#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2
#define MONO_FONT               8

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#define FS_LATIN1               0x00000001L
#define FS_LATIN2               0x00000002L
#define FS_CYRILLIC             0x00000004L
#define FS_GREEK                0x00000008L
#define FS_TURKISH              0x00000010L
#define FS_HEBREW               0x00000020L
#define FS_ARABIC               0x00000040L
#define FS_BALTIC               0x00000080L
#define FS_VIETNAMESE           0x00000100L
#define FS_THAI                 0x00010000L
#define FS_JISJAPAN             0x00020000L
#define FS_CHINESESIMP          0x00040000L
#define FS_WANSUNG              0x00080000L
#define FS_CHINESETRAD          0x00100000L
#define FS_JOHAB                0x00200000L
#define FS_SYMBOL               0x80000000L

// Font Families

#define FF_DONTCARE 	(0 << 4)
#define FF_ROMAN		(1 << 4)
#define FF_SWISS		(2 << 4)
#define FF_MODERN		(3 << 4)
#define FF_SCRIPT		(4 << 4)
#define FF_DECORATIVE	(5 << 4)

// Font Weights

#define FW_DONTCARE 	0
#define FW_THIN 		100
#define FW_EXTRALIGHT	200
#define FW_LIGHT		300
#define FW_NORMAL		400
#define FW_MEDIUM		500
#define FW_SEMIBOLD 	600
#define FW_BOLD 		700
#define FW_EXTRABOLD	800
#define FW_HEAVY		900

#define FW_ULTRALIGHT	FW_EXTRALIGHT
#define FW_REGULAR		FW_NORMAL
#define FW_DEMIBOLD 	FW_SEMIBOLD
#define FW_ULTRABOLD	FW_EXTRABOLD
#define FW_BLACK		FW_HEAVY

#define GetRValue(rgb)	((uint8)(rgb))
#define GetGValue(rgb)	((uint8)(((uint16)(rgb)) >> 8))
#define GetBValue(rgb)	((uint8)((rgb) >> 16))

#define TRANSPARENT 	1
#define OPAQUE			2

#define GM_COMPATIBLE	1
#define GM_ADVANCED 	2

#define PT_CLOSEFIGURE	0x01
#define PT_LINETO		0x02
#define PT_BEZIERTO 	0x04
#define PT_MOVETO		0x06

#define MM_TEXT 		1
#define MM_LOMETRIC 	2
#define MM_HIMETRIC 	3
#define MM_LOENGLISH	4
#define MM_HIENGLISH	5
#define MM_TWIPS		6
#define MM_ISOTROPIC	7
#define MM_ANISOTROPIC	8

#define ABSOLUTE		1
#define RELATIVE		2

// Stock Objects

#define WHITE_BRUSH 		0
#define LTGRAY_BRUSH		1
#define GRAY_BRUSH			2
#define DKGRAY_BRUSH		3
#define BLACK_BRUSH 		4
#define NULL_BRUSH			5
#define HOLLOW_BRUSH		NULL_BRUSH
#define WHITE_PEN			6
#define BLACK_PEN			7
#define NULL_PEN			8
#define OEM_FIXED_FONT		10
#define ANSI_FIXED_FONT 	11
#define ANSI_VAR_FONT		12
#define SYSTEM_FONT 		13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE 	15
#define SYSTEM_FIXED_FONT	16
#define DEFAULT_GUI_FONT	17
#define DC_BRUSH			18
#define DC_PEN				19

// Brush Styles

#define BS_SOLID			0
#define BS_NULL 			1
#define BS_HOLLOW			BS_NULL
#define BS_HATCHED			2
#define BS_PATTERN			3
#define BS_INDEXED			4
#define BS_DIBPATTERN		5
#define BS_DIBPATTERNPT 	6
#define BS_PATTERN8X8		7
#define BS_DIBPATTERN8X8	8
#define BS_MONOPATTERN		9

// Hatch Styles

#define HS_HORIZONTAL	0
#define HS_VERTICAL 	1
#define HS_FDIAGONAL	2
#define HS_BDIAGONAL	3
#define HS_CROSS		4
#define HS_DIAGCROSS	5

// Pen Styles

#define PS_SOLID		0
#define PS_DASH 		1
#define PS_DOT			2
#define PS_DASHDOT		3
#define PS_DASHDOTDOT	4
#define PS_NULL 		5
#define PS_INSIDEFRAME	6
#define PS_USERSTYLE	7
#define PS_ALTERNATE	8
#define PS_STYLE_MASK	0x0000000F

#define PS_ENDCAP_ROUND 	0x00000000
#define PS_ENDCAP_SQUARE	0x00000100
#define PS_ENDCAP_FLAT		0x00000200
#define PS_ENDCAP_MASK		0x00000F00

#define PS_JOIN_ROUND		0x00000000
#define PS_JOIN_BEVEL		0x00001000
#define PS_JOIN_MITER		0x00002000
#define PS_JOIN_MASK		0x0000F000

#define PS_COSMETIC 		0x00000000
#define PS_GEOMETRIC		0x00010000
#define PS_TYPE_MASK		0x000F0000

#define AD_COUNTERCLOCKWISE	1
#define AD_CLOCKWISE		2

struct RECTL {
	uint32 left;
	uint32 top;
	uint32 right;
	uint32 bottom;
};

struct SIZEL {
	uint32 cx;
	uint32 cy;
};

struct POINTL {
	uint32 x;
	uint32 y;
};

struct POINTS {
	uint16 x;
	uint16 y;
};

struct XFORM {
	float eM11;
	float eM12;
	float eM21;
	float eM22;
	float eDx;
	float eDy;
};

typedef int32 COLORREF;

#define LF_FACESIZE		32
#define LF_FULLFACESIZE	64

struct LOGFONTW {
	int32	lfHeight;
	int32	lfWidth;
	int32	lfEscapement;
	int32	lfOrientation;
	int32	lfWeight;
	uint8	lfItalic;
	uint8	lfUnderline;
	uint8	lfStrikeOut;
	uint8	lfCharSet;
	uint8	lfOutPrecision;
	uint8	lfClipPrecision;
	uint8	lfQuality;
	uint8	lfPitchAndFamily;
	wchar_t	lfFaceName[LF_FACESIZE];
};

struct PANOSE {
	uint8	bFamilyType;
	uint8	bSerifStyle;
	uint8	bWeight;
	uint8	bProportion;
	uint8	bContrast;
	uint8	bStrokeVariation;
	uint8	bArmStyle;
	uint8	bLetterform;
	uint8	bMidline;
	uint8	bXHeight;
};

#define ELF_VENDOR_SIZE	4

struct EXTLOGFONTW {
	LOGFONTW	elfLogFont;
	wchar_t		elfFullName[LF_FULLFACESIZE];
	wchar_t		elfStyle[LF_FACESIZE];
	uint32		elfVersion;
	uint32		elfStyleSize;
	uint32		elfMatch;
	uint32		elfReserved;
	uint8		elfVendorId[ELF_VENDOR_SIZE];
	uint32		elfCulture;
	PANOSE		elfPanose;
};

struct PALETTEENTRY {
	uint8	peRed;
	uint8	peGreen;
	uint8	peBlue;
	uint8	peFlags;
};

struct LOGPEN {
	uint32		lopnStyle;
	POINTL		lopnWidth;
	COLORREF	lopnColor;
};

struct EXTLOGPEN {
	uint32		elpPenStyle;
	uint32		elpWidth;
	uint32		elpBrushStyle;
	COLORREF	elpColor;
	int32		elpHatch;
	uint32		elpNumEntries;
	uint32		elpStyleEntry[1];
};

struct LOGBRUSH {
	uint32		lbStyle;
	COLORREF	lbColor;
	int32		lbHatch;
};

struct LOGPALETTE {
	uint16			palVersion;
	uint16			palNumEntries;
	PALETTEENTRY	palPalEntry[1];
};

struct PIXELFORMATDESCRIPTOR {
	uint16	nSize;
	uint16	nVersion;
	uint32	dwFlags;
	uint8	iPixelType;
	uint8	cColorBits;
	uint8	cRedBits;
	uint8	cRedShift;
	uint8	cGreenBits;
	uint8	cGreenShift;
	uint8	cBlueBits;
	uint8	cBlueShift;
	uint8	cAlphaBits;
	uint8	cAlphaShift;
	uint8	cAccumBits;
	uint8	cAccumRedBits;
	uint8	cAccumGreenBits;
	uint8	cAccumBlueBits;
	uint8	cAccumAlphaBits;
	uint8	cDepthBits;
	uint8	cStencilBits;
	uint8	cAuxBuffers;
	uint8	iLayerType;
	uint8	bReserved;
	uint32	dwLayerMask;
	uint32	dwVisibleMask;
	uint32	dwDamageMask;
};

struct COLORADJUSTMENT {
	uint16	caSize;
	uint16	caFlags;
	uint16	caIlluminantIndex;
	uint16	caRedGamma;
	uint16	caGreenGamma;
	uint16	caBlueGamma;
	uint16	caReferenceBlack;
	uint16	caReferenceWhite;
	int16	caContrast;
	int16	caBrightness;
	int16	caColorfulness;
	int16	caRedGreenTint;
};

struct BITMAPINFOHEADER {
	uint32	biSize;
	int32	biWidth;
	int32	biHeight;
	uint16	biPlanes;
	uint16	biBitCount;
	uint32	biCompression;
	uint32	biSizeImage;
	int32	biXPelsPerMeter;
	int32	biYPelsPerMeter;
	uint32	biClrUsed;
	uint32	biClrImportant;
};

struct TRIVERTEX {
    int32	x;
    int32	y;
    uint16	Red;
    uint16	Green;
    uint16	Blue;
    uint16	Alpha;
};

struct CIEXYZ {
	uint32 ciexyzX;
	uint32 ciexyzY;
	uint32 ciexyzZ;
};

struct CIEXYZTRIPLE {
	CIEXYZ ciexyzRed;
	CIEXYZ ciexyzGreen;
	CIEXYZ ciexyzBlue;
};

struct LOGCOLORSPACEW {
	uint32			lcsSignature;
	uint32			lcsVersion;
	uint32			lcsSize;
	int32			lcsCSType;
	int32			lcsIntent;
	CIEXYZTRIPLE	lcsEndpoints;
	uint32			lcsGammaRed;
	uint32			lcsGammaGreen;
	uint32			lcsGammaBlue;
	wchar_t			lcsFilename[MAX_PATH];
};

/* Enhanced Metafile structures */

struct ENHMETARECORD {
	uint32	iType;
	uint32	nSize;
	uint32	dParm[1];
};

struct ENHMETAHEADER {
	uint32	iType;
	uint32	nSize;
	RECTL	rclBounds;
	RECTL	rclFrame;
	uint32	dSignature;
	uint32	nVersion;
	uint32	nBytes;
	uint32	nRecords;
	uint16	nHandles;
	uint16	sReserved;
	uint32	nDescription;
	uint32	offDescription;
	uint32	nPalEntries;
	SIZEL	szlDevice;
	SIZEL	szlMillimeters;
	uint32	cbPixelFormat;
	uint32	offPixelFormat;
	uint32	bOpenGL;
};

// Enhanced metafile constants.

#if (!__MWERKS__)
#define ENHMETA_SIGNATURE	0x464D4520
#else
#define ENHMETA_SIGNATURE	0x20454D46
#endif

#define ENHMETA_STOCK_OBJECT	0x80000000

// Enhanced metafile record types.

#define EMR_HEADER					1
#define EMR_POLYBEZIER				2
#define EMR_POLYGON 				3
#define EMR_POLYLINE				4
#define EMR_POLYBEZIERTO			5
#define EMR_POLYLINETO				6
#define EMR_POLYPOLYLINE			7
#define EMR_POLYPOLYGON 			8
#define EMR_SETWINDOWEXTEX			9
#define EMR_SETWINDOWORGEX			10
#define EMR_SETVIEWPORTEXTEX		11
#define EMR_SETVIEWPORTORGEX		12
#define EMR_SETBRUSHORGEX			13
#define EMR_EOF 					14
#define EMR_SETPIXELV				15
#define EMR_SETMAPPERFLAGS			16
#define EMR_SETMAPMODE				17
#define EMR_SETBKMODE				18
#define EMR_SETPOLYFILLMODE 		19
#define EMR_SETROP2 				20
#define EMR_SETSTRETCHBLTMODE		21
#define EMR_SETTEXTALIGN			22
#define EMR_SETCOLORADJUSTMENT		23
#define EMR_SETTEXTCOLOR			24
#define EMR_SETBKCOLOR				25
#define EMR_OFFSETCLIPRGN			26
#define EMR_MOVETOEX				27
#define EMR_SETMETARGN				28
#define EMR_EXCLUDECLIPRECT 		29
#define EMR_INTERSECTCLIPRECT		30
#define EMR_SCALEVIEWPORTEXTEX		31
#define EMR_SCALEWINDOWEXTEX		32
#define EMR_SAVEDC					33
#define EMR_RESTOREDC				34
#define EMR_SETWORLDTRANSFORM		35
#define EMR_MODIFYWORLDTRANSFORM	36
#define EMR_SELECTOBJECT			37
#define EMR_CREATEPEN				38
#define EMR_CREATEBRUSHINDIRECT 	39
#define EMR_DELETEOBJECT			40
#define EMR_ANGLEARC				41
#define EMR_ELLIPSE 				42
#define EMR_RECTANGLE				43
#define EMR_ROUNDRECT				44
#define EMR_ARC 					45
#define EMR_CHORD					46
#define EMR_PIE 					47
#define EMR_SELECTPALETTE			48
#define EMR_CREATEPALETTE			49
#define EMR_SETPALETTEENTRIES		50
#define EMR_RESIZEPALETTE			51
#define EMR_REALIZEPALETTE			52
#define EMR_EXTFLOODFILL			53
#define EMR_LINETO					54
#define EMR_ARCTO					55
#define EMR_POLYDRAW				56
#define EMR_SETARCDIRECTION 		57
#define EMR_SETMITERLIMIT			58
#define EMR_BEGINPATH				59
#define EMR_ENDPATH 				60
#define EMR_CLOSEFIGURE 			61
#define EMR_FILLPATH				62
#define EMR_STROKEANDFILLPATH		63
#define EMR_STROKEPATH				64
#define EMR_FLATTENPATH 			65
#define EMR_WIDENPATH				66
#define EMR_SELECTCLIPPATH			67
#define EMR_ABORTPATH				68

#define EMR_GDICOMMENT				70
#define EMR_FILLRGN 				71
#define EMR_FRAMERGN				72
#define EMR_INVERTRGN				73
#define EMR_PAINTRGN				74
#define EMR_EXTSELECTCLIPRGN		75
#define EMR_BITBLT					76
#define EMR_STRETCHBLT				77
#define EMR_MASKBLT 				78
#define EMR_PLGBLT					79
#define EMR_SETDIBITSTODEVICE		80
#define EMR_STRETCHDIBITS			81
#define EMR_EXTCREATEFONTINDIRECTW	82
#define EMR_EXTTEXTOUTA 			83
#define EMR_EXTTEXTOUTW 			84
#define EMR_POLYBEZIER16			85
#define EMR_POLYGON16				86
#define EMR_POLYLINE16				87
#define EMR_POLYBEZIERTO16			88
#define EMR_POLYLINETO16			89
#define EMR_POLYPOLYLINE16			90
#define EMR_POLYPOLYGON16			91
#define EMR_POLYDRAW16				92
#define EMR_CREATEMONOBRUSH 		93
#define EMR_CREATEDIBPATTERNBRUSHPT 94
#define EMR_EXTCREATEPEN			95
#define EMR_POLYTEXTOUTA			96
#define EMR_POLYTEXTOUTW			97

#define EMR_SETICMMODE				98
#define EMR_CREATECOLORSPACE		99
#define EMR_SETCOLORSPACE			100
#define EMR_DELETECOLORSPACE		101
#define EMR_GLSRECORD				102
#define EMR_GLSBOUNDEDRECORD		103
#define EMR_PIXELFORMAT 			104

#define EMR_DRAWESCAPE				105
#define EMR_EXTESCAPE				106
#define EMR_STARTDOC				107
#define EMR_SMALLTEXTOUT			108
#define EMR_FORCEUFIMAPPING 		109
#define EMR_NAMEDESCAPE 			110
#define EMR_COLORCORRECTPALETTE 	111
#define EMR_SETICMPROFILEA			112
#define EMR_SETICMPROFILEW			113
#define EMR_ALPHABLEND				114
#define EMR_ALPHADIBBLEND			115
#define EMR_TRANSPARENTBLT			116
#define EMR_TRANSPARENTDIB			117
#define EMR_GRADIENTFILL			118
#define EMR_SETLINKEDUFIS			119
#define EMR_SETTEXTJUSTIFICATION	120

// Base record type for the enhanced metafile.

struct EMR {
	uint32	 iType;
	uint32	 nSize;
};

// Base text record type for the enhanced metafile.

struct EMRTEXT {
	POINTL	ptlReference;
	uint32	nChars;
	uint32	offString;
	uint32	fOptions;
	RECTL	rcl;
	uint32	offDx;
};

// Record structures for the enhanced metafile.

struct ABORTPATH {
	EMR		emr;
};

typedef ABORTPATH	EMRABORTPATH;
typedef ABORTPATH	EMRBEGINPATH;
typedef ABORTPATH	EMRENDPATH;
typedef ABORTPATH	EMRCLOSEFIGURE;
typedef ABORTPATH	EMRFLATTENPATH;
typedef ABORTPATH	EMRWIDENPATH;
typedef ABORTPATH	EMRSETMETARGN;
typedef ABORTPATH	EMRSAVEDC;
typedef ABORTPATH	EMRREALIZEPALETTE;

struct EMRSELECTCLIPPATH {
	EMR 	emr;
	uint32	 iMode;
};

typedef EMRSELECTCLIPPATH	EMRSETBKMODE;
typedef EMRSELECTCLIPPATH	EMRSETMAPMODE;
typedef EMRSELECTCLIPPATH	EMRSETPOLYFILLMODE;
typedef EMRSELECTCLIPPATH	EMRSETROP2;
typedef EMRSELECTCLIPPATH	EMRSETSTRETCHBLTMODE;
typedef EMRSELECTCLIPPATH	EMRSETICMMODE;
typedef EMRSELECTCLIPPATH	EMRSETTEXTALIGN;

struct EMRSETMITERLIMIT {
	EMR 	emr;
	float	eMiterLimit;
};

struct EMRRESTOREDC {
	EMR 	emr;
	int32	iRelative;
};

struct EMRSETARCDIRECTION {
	EMR 	emr;
	uint32	iArcDirection;
};

struct EMRSETMAPPERFLAGS {
	EMR 	emr;
	uint32	dwFlags;
};

struct EMRSETTEXTCOLOR {
	EMR 		emr;
	COLORREF	crColor;
};

typedef EMRSETTEXTCOLOR	EMRSETBKCOLOR;

struct EMRSELECTOBJECT {
	EMR 	emr;
	uint32	ihObject;
};

typedef EMRSELECTOBJECT EMRDELETEOBJECT;

struct EMRSELECTCOLORSPACE {
	EMR 	emr;
	uint32	ihCS;
};

typedef EMRSELECTCOLORSPACE EMRDELETECOLORSPACE;

struct EMRSELECTPALETTE {
	EMR 	emr;
	uint32	ihPal;
};

struct EMRRESIZEPALETTE {
	EMR 	emr;
	uint32	ihPal;
	uint32	cEntries;
};

struct EMRSETPALETTEENTRIES {
	EMR 	emr;
	uint32	ihPal;
	uint32	iStart;
	uint32	cEntries;
	PALETTEENTRY aPalEntries[1];
};

struct EMRSETCOLORADJUSTMENT {
	EMR 	emr;
	COLORADJUSTMENT ColorAdjustment;
};

struct EMRGDICOMMENT {
	EMR 	emr;
	uint32	 cbData;
	uint8	 Data[1];
};

struct EMREOF {
	EMR 	emr;
	uint32	nPalEntries;
	uint32	offPalEntries;
	uint32	nSizeLast;
};

struct EMRLINETO {
	EMR 	emr;
	POINTL	ptl;
};

typedef EMRLINETO	EMRMOVETOEX;

struct EMROFFSETCLIPRGN {
	EMR 	emr;
	POINTL	ptlOffset;
};

struct EMRFILLPATH {
	EMR 	emr;
	RECTL	rclBounds;
};

typedef EMRFILLPATH EMRSTROKEANDFILLPATH;
typedef EMRFILLPATH EMRSTROKEPATH;

struct EMREXCLUDECLIPRECT {
	EMR 	emr;
	RECTL	rclClip;
};

typedef EMREXCLUDECLIPRECT	EMRINTERSECTCLIPRECT;

struct EMRSETVIEWPORTORGEX {
	EMR 	emr;
	POINTL	ptlOrigin;
};

typedef EMRSETVIEWPORTORGEX EMRSETWINDOWORGEX;
typedef EMRSETVIEWPORTORGEX EMRSETBRUSHORGEX;

struct EMRSETVIEWPORTEXTEX {
	EMR 	emr;
	SIZEL	szlExtent;
};

typedef EMRSETVIEWPORTEXTEX EMRSETWINDOWEXTEX;

struct EMRSCALEVIEWPORTEXTEX {
	EMR 	emr;
	int32	xNum;
	int32	xDenom;
	int32	yNum;
	int32	yDenom;
};

typedef EMRSCALEVIEWPORTEXTEX EMRSCALEWINDOWEXTEX;

struct EMRSETWORLDTRANSFORM {
	EMR 	emr;
	XFORM	xform;
};

struct EMRMODIFYWORLDTRANSFORM {
	EMR 	emr;
	XFORM	xform;
	uint32	iMode;
};

struct EMRSETPIXELV {
	EMR 		emr;
	POINTL		ptlPixel;
	COLORREF	crColor;
};

struct EMREXTFLOODFILL {
	EMR 		emr;
	POINTL		ptlStart;
	COLORREF	crColor;
	uint32	 	iMode;
};

struct EMRELLIPSE {
	EMR 	emr;
	RECTL	rclBox;
};

typedef EMRELLIPSE	EMRRECTANGLE;

struct EMRROUNDRECT {
	EMR 	emr;
	RECTL	rclBox;
	SIZEL	szlCorner;
};

struct EMRARC {
	EMR 	emr;
	RECTL	rclBox;
	POINTL	ptlStart;
	POINTL	ptlEnd;
};

typedef EMRARC	EMRARCTO;
typedef EMRARC	EMRCHORD;
typedef EMRARC	EMRPIE;

struct EMRANGLEARC {
	EMR 	emr;
	POINTL	ptlCenter;
	uint32	 nRadius;
	float	eStartAngle;
	float	eSweepAngle;
};

struct EMRPOLYLINE {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cptl;
	POINTL	aptl[1];
};

typedef EMRPOLYLINE EMRPOLYBEZIER;
typedef EMRPOLYLINE EMRPOLYGON;
typedef EMRPOLYLINE EMRPOLYBEZIERTO;
typedef EMRPOLYLINE EMRPOLYLINETO;

struct EMRPOLYLINE16 {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cpts;
	POINTS	apts[1];
};

typedef EMRPOLYLINE16	EMRPOLYBEZIER16;
typedef EMRPOLYLINE16	EMRPOLYGON16;
typedef EMRPOLYLINE16	EMRPOLYBEZIERTO16;
typedef EMRPOLYLINE16	EMRPOLYLINETO16;

struct EMRPOLYDRAW {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cptl;
	POINTL	aptl[1];
	uint8	abTypes[1];
};

struct EMRPOLYDRAW16 {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cpts;
	POINTS	apts[1];
	uint8	abTypes[1];
};

struct EMRPOLYPOLYLINE {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	nPolys;
	uint32	cptl;
	uint32	aPolyCounts[1];
	POINTL	aptl[1];
};

typedef EMRPOLYPOLYLINE EMRPOLYPOLYGON;

struct EMRPOLYPOLYLINE16 {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	nPolys;
	uint32	cpts;
	uint32	aPolyCounts[1];
	POINTS	apts[1];
};

typedef EMRPOLYPOLYLINE16	EMRPOLYPOLYGON16;

struct EMRINVERTRGN {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cbRgnData;
	uint8	RgnData[1];
};

typedef EMRINVERTRGN	EMRPAINTRGN;

struct EMRFILLRGN {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cbRgnData;
	uint32	ihBrush;
	uint8	RgnData[1];
};

struct EMRFRAMERGN {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cbRgnData;
	uint32	ihBrush;
	SIZEL	szlStroke;
	uint8	RgnData[1];
};

struct EMRGRADIENTFILL {
	EMR 	  emr;
	RECTL	  rclBounds;
	uint32	  nVer;
	uint32	  nTri;
	uint32	  ulMode;
	TRIVERTEX Ver[1];
};

struct EMREXTSELECTCLIPRGN {
	EMR 	emr;
	uint32	cbRgnData; 	// Size of region data in bytes
	uint32	iMode;
	uint8	RgnData[1];
};

struct EMREXTTEXTOUTA {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	iGraphicsMode;
	float	exScale;
	float	eyScale;
	EMRTEXT emrtext;
};

typedef EMREXTTEXTOUTA	EMREXTTEXTOUTW;

struct EMRPOLYTEXTOUTA {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	iGraphicsMode;
	float	exScale;
	float	eyScale;
	int32	cStrings;
	EMRTEXT aemrtext[1];
};

typedef EMRPOLYTEXTOUTA EMRPOLYTEXTOUTW;

struct EMRBITBLT {
	EMR 		emr;
	RECTL		rclBounds;
	int32		xDest;
	int32		yDest;
	int32		cxDest;
	int32		cyDest;
	uint32		dwRop;
	int32		xSrc;
	int32		ySrc;
	XFORM		xformSrc;
	COLORREF	crBkColorSrc;
	uint32		iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
};

struct EMRSTRETCHBLT {
	EMR 		emr;
	RECTL		rclBounds;
	int32		xDest;
	int32		yDest;
	int32		cxDest;
	int32		cyDest;
	uint32		dwRop;
	int32		xSrc;
	int32		ySrc;
	XFORM		xformSrc;
	COLORREF	crBkColorSrc;
	uint32		iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
	int32		cxSrc;
	int32		cySrc;
};

struct EMRMASKBLT {
	EMR 		emr;
	RECTL		rclBounds;
	int32		xDest;
	int32		yDest;
	int32		cxDest;
	int32		cyDest;
	uint32		dwRop;
	int32		xSrc;
	int32		ySrc;
	XFORM		xformSrc;
	COLORREF 	crBkColorSrc;
	uint32	 	iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
	int32		xMask;
	int32		yMask;
	uint32		iUsageMask;
	uint32		offBmiMask;
	uint32		cbBmiMask;
	uint32		offBitsMask;
	uint32		cbBitsMask;
};

struct EMRPLGBLT {
	EMR 		emr;
	RECTL		rclBounds;
	POINTL		aptlDest[3];
	int32		xSrc;
	int32		ySrc;
	int32		cxSrc;
	int32		cySrc;
	XFORM		xformSrc;
	COLORREF	crBkColorSrc;
	uint32		iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
	int32		xMask;
	int32		yMask;
	uint32		iUsageMask;
	uint32		offBmiMask;
	uint32		cbBmiMask;
	uint32		offBitsMask;
	uint32		cbBitsMask;
};

struct EMRSETDIBITSTODEVICE {
	EMR 	emr;
	RECTL	rclBounds;
	int32	xDest;
	int32	yDest;
	int32	xSrc;
	int32	ySrc;
	int32	cxSrc;
	int32	cySrc;
	uint32	offBmiSrc;
	uint32	cbBmiSrc;
	uint32	offBitsSrc;
	uint32	cbBitsSrc;
	uint32	iUsageSrc;
	uint32	iStartScan;
	uint32	 cScans;
};

struct EMRSTRETCHDIBITS {
	EMR 	emr;
	RECTL	rclBounds;
	int32	xDest;
	int32	yDest;
	int32	xSrc;
	int32	ySrc;
	int32	cxSrc;
	int32	cySrc;
	uint32	offBmiSrc;
	uint32	cbBmiSrc;
	uint32	offBitsSrc;
	uint32	cbBitsSrc;
	uint32	iUsageSrc;
	uint32	dwRop;
	int32	cxDest;
	int32	cyDest;
};

struct EMRALPHABLEND {
	EMR 		emr;
	RECTL		rclBounds;
	int32		xDest;
	int32		yDest;
	int32		cxDest;
	int32		cyDest;
	uint32		dwRop;
	int32		xSrc;
	int32		ySrc;
	XFORM		xformSrc;
	COLORREF	crBkColorSrc;
	uint32		iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
	int32		cxSrc;
	int32		cySrc;
};

struct EMRTRANSPARENTBLT {
	EMR 		emr;
	RECTL		rclBounds;
	int32		xDest;
	int32		yDest;
	int32		cxDest;
	int32		cyDest;
	uint32		dwRop;
	int32		xSrc;
	int32		ySrc;
	XFORM		xformSrc;
	COLORREF	crBkColorSrc;
	uint32		iUsageSrc;
	uint32		offBmiSrc;
	uint32		cbBmiSrc;
	uint32		offBitsSrc;
	uint32		cbBitsSrc;
	int32		cxSrc;
	int32		cySrc;
};

struct EMREXTCREATEFONTINDIRECTW {
	EMR 		emr;
	uint32		ihFont;
	EXTLOGFONTW	elfw;
};

struct EMRCREATEPALETTE {
	EMR 		emr;
	uint32	 	ihPal;
	LOGPALETTE	lgpl;
};

struct EMRCREATECOLORSPACE {
	EMR 			emr;
	uint32			ihCS;
	LOGCOLORSPACEW	lcs;
};

struct EMRCREATEPEN {
	EMR 	emr;
	uint32	ihPen;
	LOGPEN	lopn;
};

struct EMREXTCREATEPEN {
	EMR 		emr;
	uint32	 	ihPen;
	uint32	 	offBmi;
	uint32	 	cbBmi;
	uint32	 	offBits;
	uint32	 	cbBits;
	EXTLOGPEN	elp;
};

struct EMRCREATEBRUSHINDIRECT {
	EMR 		emr;
	uint32		ihBrush;
	LOGBRUSH	lb;
};

struct EMRCREATEMONOBRUSH {
	EMR 	emr;
	uint32	ihBrush;
	uint32	iUsage;
	uint32	offBmi;
	uint32	cbBmi;
	uint32	offBits;
	uint32	cbBits;
};

typedef EMRCREATEMONOBRUSH	EMRCREATEDIBPATTERNBRUSHPT;

struct EMRFORMAT {
	uint32	dSignature;
	uint32	nVersion;
	uint32	cbData;
	uint32	offData;
};

struct EMRGLSRECORD {
	EMR 	emr;
	uint32	cbData;
	uint8	Data[1];
};

struct EMRGLSBOUNDEDRECORD {
	EMR 	emr;
	RECTL	rclBounds;
	uint32	cbData;
	uint8	Data[1];
};

struct EMRPIXELFORMAT {
	EMR 	emr;
	PIXELFORMATDESCRIPTOR pfd;
};

#define SETICMPROFILE_EMBEDED	0x00000001

struct EMRSETICMPROFILE {
	EMR 	emr;
	uint32	dwFlags;
	uint32	cbName;
	uint32	cbData;
	uint8	Data[1];
};

#define GDICOMMENT_IDENTIFIER           0x43494447
#define GDICOMMENT_WINDOWS_METAFILE     0x80000001
#define GDICOMMENT_BEGINGROUP           0x00000002
#define GDICOMMENT_ENDGROUP             0x00000003
#define GDICOMMENT_MULTIFORMATS         0x40000004
#define EPS_SIGNATURE                   0x46535045

#endif	// __GDI_H
