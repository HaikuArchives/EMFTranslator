/*
 * dbgmsg.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <cstdio>
#include <cstdarg>

#include <Message.h>
#include <File.h>
#include <Directory.h>
#include <fs_attr.h>

#include "gdi.h"
#include "dbgmsg.h"

#ifdef DBG

#if (!__MWERKS__ || defined(MSIPL_USING_NAMESPACE))
using namespace std;
#else 
#define std
#endif

void write_debug_stream(const char *format, ...)
{
	va_list	ap;
	va_start(ap, format);
#if 0
	FILE *f = fopen("/boot/home/emf_log.txt", "aw+");
	vfprintf(f, format, ap);
	fclose(f);
#else
	vprintf(format, ap);
#endif
	va_end(ap);
}


void DUMP_BFILE(BFile *in, const char *path)
{
	off_t size;
	if (B_NO_ERROR == in->GetSize(&size)) {
		uchar *buffer = new uchar[size];
		in->Read(buffer, size);
		BFile out(path, B_WRITE_ONLY | B_CREATE_FILE);
		out.Write(buffer, size);
		in->Seek(0, SEEK_SET);
		delete [] buffer;
	}
}

void DUMP_BMESSAGE(BMessage *msg)
{
	uint32 i;
	int32 j;
	char *name  = "";
	uint32 type = 0;
	int32 count = 0;

	DBGMSG(("\t************ START - DUMP BMessage ***********\n"));
	DBGMSG(("\taddress: 0x%x\n", (int)msg));
	if (!msg)
		return;	

#if (!__MWERKS__)
	DBGMSG(("\tmsg->what: %c%c%c%c\n",
		*((char *)&msg->what + 3),
		*((char *)&msg->what + 2),
		*((char *)&msg->what + 1),
		*((char *)&msg->what + 0)));
#else
	DBGMSG(("\tmsg->what: %c%c%c%c\n",
		*((char *)&msg->what + 0),
		*((char *)&msg->what + 1),
		*((char *)&msg->what + 2),
		*((char *)&msg->what + 3)));
#endif

	for (i= 0; msg->GetInfo(B_ANY_TYPE, i, &name, &type, &count) == B_OK; i++) {
		switch (type) {
		case B_BOOL_TYPE:
			for (j = 0; j < count; j++) {
				bool aBool;
				aBool = msg->FindBool(name, j);
				DBGMSG(("\t%s, B_BOOL_TYPE[%d]: %s\n",
					name, j, aBool ? "true" : "false"));
			}
			break;

		case B_INT8_TYPE:
			for (j = 0; j < count; j++) {
				int8 anInt8;
				msg->FindInt8(name, j, &anInt8);
				DBGMSG(("\t%s, B_INT8_TYPE[%d]: %d\n", name, j, (int)anInt8));
			}
			break;

		case B_INT16_TYPE:
			for (j = 0; j < count; j++) {
				int16 anInt16;
				msg->FindInt16(name, j, &anInt16);
				DBGMSG(("\t%s, B_INT16_TYPE[%d]: %d\n", name, j, (int)anInt16));
			}
			break;

		case B_INT32_TYPE:
			for (j = 0; j < count; j++) {
				int32 anInt32;
				msg->FindInt32(name, j, &anInt32);
				DBGMSG(("\t%s, B_INT32_TYPE[%d]: %d\n", name, j, (int)anInt32));
			}
			break;

		case B_INT64_TYPE:
			for (j = 0; j < count; j++) {
				int64 anInt64;
				msg->FindInt64(name, j, &anInt64);
				DBGMSG(("\t%s, B_INT64_TYPE[%d]: %d\n", name, j, (int)anInt64));
			}
			break;

		case B_FLOAT_TYPE:
			for (j = 0; j < count; j++) {
				float aFloat;
				msg->FindFloat(name, j, &aFloat);
				DBGMSG(("\t%s, B_FLOAT_TYPE[%d]: %f\n", name, j, aFloat));
			}
			break;

		case B_DOUBLE_TYPE:
			for (j = 0; j < count; j++) {
				double aDouble;
				msg->FindDouble(name, j, &aDouble);
				DBGMSG(("\t%s, B_DOUBLE_TYPE[%d]: %f\n", name, j, (float)aDouble));
			}
			break;

		case B_STRING_TYPE:
			for (j = 0; j < count; j++) {
				const char *string;
				msg->FindString(name, j, &string);
				DBGMSG(("\t%s, B_STRING_TYPE[%d]: %s\n", name, j, string));
			}
			break;

		case B_POINT_TYPE:
			for (j = 0; j < count; j++) {
				BPoint aPoint;
				msg->FindPoint(name, j, &aPoint);
				DBGMSG(("\t%s, B_POINT_TYPE[%d]: %f, %f\n",
					name, j, aPoint.x, aPoint.y));
			}
			break;

		case B_RECT_TYPE:
			for (j = 0; j < count; j++) {
				BRect aRect;
				msg->FindRect(name, j, &aRect);
				DBGMSG(("\t%s, B_RECT_TYPE[%d]: %f, %f, %f, %f\n",
					name, j, aRect.left, aRect.top, aRect.right, aRect.bottom));
			}
			break;

		case B_REF_TYPE:
		case B_MESSAGE_TYPE:
		case B_MESSENGER_TYPE:
		case B_POINTER_TYPE:
			DBGMSG(("\t%s, 0x%x, count: %d\n",
				name ? name : "(null)", type, count));
			break;
		default:
			DBGMSG(("\t%s, 0x%x, count: %d\n",
				name ? name : "(null)", type, count));
			break;
		}

		name  = "";
		type  = 0;
		count = 0;
	}
	DBGMSG(("\t************ END - DUMP BMessage ***********\n"));
}

void DUMP_BDIRECTORY(BDirectory *dir)
{
	char buffer1[256];
	char buffer2[256];
	attr_info info;
	int32 i;
	float f;
	BRect rc;
	bool b;

	DBGMSG(("\t************ STRAT - DUMP BDirectory ***********\n"));

	dir->RewindAttrs();
	while (dir->GetNextAttrName(buffer1) == B_NO_ERROR) {
		dir->GetAttrInfo(buffer1, &info);
		switch (info.type) {
		case B_ASCII_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, buffer2, sizeof(buffer2));
			DBGMSG(("\t%s, B_ASCII_TYPE: %s\n", buffer1, buffer2));
			break;
		case B_STRING_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, buffer2, sizeof(buffer2));
			DBGMSG(("\t%s, B_STRING_TYPE: %s\n", buffer1, buffer2));
			break;
		case B_INT32_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, &i, sizeof(i));
			DBGMSG(("\t%s, B_INT32_TYPE: %d\n", buffer1, i));
			break;
		case B_FLOAT_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, &f, sizeof(f));
			DBGMSG(("\t%s, B_FLOAT_TYPE: %f\n", buffer1, f));
			break;
		case B_RECT_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, &rc, sizeof(rc));
			DBGMSG(("\t%s, B_RECT_TYPE: %f, %f, %f, %f\n", buffer1, rc.left, rc.top, rc.right, rc.bottom));
			break;
		case B_BOOL_TYPE:
			dir->ReadAttr(buffer1, info.type, 0, &b, sizeof(b));
			DBGMSG(("\t%s, B_BOOL_TYPE: %d\n", buffer1, (int)b));
			break;
		default:
			DBGMSG(("\t%s, %c%c%c%c\n",
				buffer1,
				*((char *)&info.type + 3),
				*((char *)&info.type + 2),
				*((char *)&info.type + 1),
				*((char *)&info.type + 0)));
			break;
		}
	}

	DBGMSG(("\t************ END - DUMP BDirectory ***********\n"));
}

void DUMP_ENHMETAHEADER(ENHMETAHEADER *pemh)
{
	DBGMSG(("\tiType          = %d\n", pemh->iType));
	DBGMSG(("\tnSize          = %d\n", pemh->nSize));
	DBGMSG(("\trclBounds      = %d, %d, %d, %d\n", pemh->rclBounds.left, pemh->rclBounds.top, pemh->rclBounds.right, pemh->rclBounds.bottom));
	DBGMSG(("\trclFrame       = %d, %d, %d, %d\n", pemh->rclFrame.left, pemh->rclFrame.top, pemh->rclFrame.right, pemh->rclFrame.bottom));
	DBGMSG(("\tdSignature     = 0x%x\n", pemh->dSignature));
	DBGMSG(("\tnVersion       = 0x%x\n", pemh->nVersion));
	DBGMSG(("\tnBytes         = %d\n", pemh->nBytes));
	DBGMSG(("\tnRecords       = %d\n", pemh->nRecords));
	DBGMSG(("\tnHandles       = %d\n", pemh->nHandles));
	DBGMSG(("\tnDescription   = %d\n", pemh->nDescription));
	DBGMSG(("\toffDescription = %d\n", pemh->offDescription));
	DBGMSG(("\tnPalEntries    = %d\n", pemh->nPalEntries));
	DBGMSG(("\tszlDevice      = %d, %d\n", pemh->szlDevice.cx, pemh->szlDevice.cy));
	DBGMSG(("\tszlMillimeters = %d, %d\n", pemh->szlMillimeters.cx, pemh->szlMillimeters.cy));
	DBGMSG(("\tcbPixelFormat  = %d\n", pemh->cbPixelFormat));
	DBGMSG(("\toffPixelFormat = %d\n", pemh->offPixelFormat));
	DBGMSG(("\tbOpenGL        = %d\n", pemh->bOpenGL));
}

void DUMP_EMRTEXT(EMRTEXT *emrtext)
{
	DBGMSG(("\tEMRTEXT::ptlReference = %d, %d\n", emrtext->ptlReference.x, emrtext->ptlReference.y));
	DBGMSG(("\tEMRTEXT::nChars       = %d\n", emrtext->nChars));
	DBGMSG(("\tEMRTEXT::offString    = %d\n", emrtext->offString));
	DBGMSG(("\tEMRTEXT::fOptions     = %d\n", emrtext->fOptions));
	DBGMSG(("\tEMRTEXT::rcl          = %d, %d, %d, %d\n", emrtext->rcl.left, emrtext->rcl.top, emrtext->rcl.right, emrtext->rcl.bottom));
	DBGMSG(("\tEMRTEXT::nChars       = %d\n", emrtext->nChars));
	DBGMSG(("\tEMRTEXT::offDx        = %d\n", emrtext->offDx));
}

void DUMP_LOGFONT(LOGFONTW *lf)
{
	DBGMSG(("\tLOGFONT::lfHeight         = %d\n", lf->lfHeight));
	DBGMSG(("\tLOGFONT::lfWidth          = %d\n", lf->lfWidth));
	DBGMSG(("\tLOGFONT::lfEscapement     = %d\n", lf->lfEscapement));
	DBGMSG(("\tLOGFONT::lfOrientation    = %d\n", lf->lfOrientation));
	DBGMSG(("\tLOGFONT::lfWeight         = %d\n", lf->lfWeight));
	DBGMSG(("\tLOGFONT::lfItalic         = %d\n", (int)lf->lfItalic));
	DBGMSG(("\tLOGFONT::lfUnderline      = %d\n", (int)lf->lfUnderline));
	DBGMSG(("\tLOGFONT::lfStrikeOut      = %d\n", (int)lf->lfStrikeOut));
	DBGMSG(("\tLOGFONT::lfCharSet        = %d\n", (int)lf->lfCharSet));
	DBGMSG(("\tLOGFONT::lfOutPrecision   = %d\n", (int)lf->lfOutPrecision));
	DBGMSG(("\tLOGFONT::lfClipPrecision  = %d\n", (int)lf->lfClipPrecision));
	DBGMSG(("\tLOGFONT::lfQuality        = %d\n", (int)lf->lfQuality));
	DBGMSG(("\tLOGFONT::lfPitchAndFamily = %d\n", (int)lf->lfPitchAndFamily));
//	DBGMSG(("\tLOGFONT::lfFaceName        = %s\n", ansiString(lf->lfFaceName)));
}

void DUMP_EXTLOGFONT(EXTLOGFONTW *elf)
{
	DUMP_LOGFONT(&elf->elfLogFont);
//	DBGMSG(("\tEXTLOGFONT::elfFullName  = %s\n", ansiString(elf->elfFullName));
//	DBGMSG(("\tEXTLOGFONT::elfFullName  = %s\n", ansiString(elf->elfStyle));
	DBGMSG(("\tEXTLOGFONT::elfVersion   = %d\n", elf->elfVersion));
	DBGMSG(("\tEXTLOGFONT::elfStyleSize = %d\n", elf->elfStyleSize));
	DBGMSG(("\tEXTLOGFONT::elfMatch     = %d\n", elf->elfMatch));
	DBGMSG(("\tEXTLOGFONT::elfReserved  = %d\n", elf->elfReserved));
	DBGMSG(("\tEXTLOGFONT::elfVendorId  = 0x%x\n", *((uint32 *)&elf->elfVendorId[0])));
	DBGMSG(("\tEXTLOGFONT::elfCulture   = %d\n", elf->elfCulture));
//	DBGMSG(("\tEXTLOGFONT::elfPanose    = %d\n", elf->elfPanose));
}

void DUMP_LOGPEN(LOGPEN *plp)
{
	DBGMSG(("\tLOGPEN::lopnStyle = "));
	switch (plp->lopnStyle & PS_STYLE_MASK) {
	case PS_SOLID:
		DBGMSG(("PS_SOLID"));
		break;
	case PS_DASH:
		DBGMSG(("PS_DASH"));
		break;
	case PS_DOT:
		DBGMSG(("PS_DOT"));
		break;
	case PS_DASHDOT:
		DBGMSG(("PS_DASHDOT"));
		break;
	case PS_DASHDOTDOT:
		DBGMSG(("PS_DASHDOTDOT"));
		break;
	case PS_NULL:
		DBGMSG(("PS_NULL"));
		break;
	case PS_INSIDEFRAME:
		DBGMSG(("PS_INSIDEFRAME"));
		break;
	case PS_USERSTYLE:
		DBGMSG(("PS_USERSTYLE"));
		break;
	case PS_ALTERNATE:
		DBGMSG(("PS_ALTERNATE"));
		break;
	}
/*
	switch (plp->lopnStyle & PS_ENDCAP_MASK) {
	case PS_ENDCAP_ROUND:
		DBGMSG((", PS_ENDCAP_ROUND"));
		break;
	case PS_ENDCAP_SQUARE:
		DBGMSG((", PS_ENDCAP_SQUARE"));
		break;
	case PS_ENDCAP_FLAT:
		DBGMSG((", PS_ENDCAP_FLAT"));
		break;
	}
	switch (plp->lopnStyle & PS_JOIN_MASK) {
	case PS_JOIN_ROUND:
		DBGMSG((", PS_JOIN_ROUND"));
		break;
	case PS_JOIN_BEVEL:
		DBGMSG((", PS_JOIN_BEVEL"));
		break;
	case PS_JOIN_MITER:
		DBGMSG((", PS_JOIN_MITER"));
		break;
	}
	switch (plp->lopnStyle & PS_TYPE_MASK) {
	case PS_COSMETIC:
		DBGMSG((", PS_COSMETIC"));
		break;
	case PS_GEOMETRIC:
		DBGMSG((", PS_GEOMETRIC"));
		break;
	}
*/
	DBGMSG(("\n"));
	DBGMSG(("\tLOGPEN::lopnWidth = %d, %d\n", plp->lopnWidth.x, plp->lopnWidth.y));
	DBGMSG(("\tLOGPEN::lopnColor = 0x%x\n", plp->lopnColor));
}

void DUMP_LOGBRUSH(LOGBRUSH *plb)
{
	DBGMSG(("\tLOGBRUSH::lbStyle = "));

	switch (plb->lbStyle) {
	case BS_SOLID:
		DBGMSG(("BS_SOLID"));
		break;
	case BS_HOLLOW:
		DBGMSG(("BS_HOLLOW"));
		break;
	case BS_HATCHED:
		DBGMSG(("BS_HATCHED"));
		break;
	case BS_PATTERN:
		DBGMSG(("BS_PATTERN"));
		break;
	case BS_INDEXED:
		DBGMSG(("BS_INDEXED"));
		break;
	case BS_DIBPATTERN:
		DBGMSG(("BS_DIBPATTERN"));
		break;
	case BS_DIBPATTERNPT:
		DBGMSG(("BS_DIBPATTERNPT"));
		break;
	case BS_PATTERN8X8:
		DBGMSG(("BS_PATTERN8X8"));
		break;
	case BS_DIBPATTERN8X8:
		DBGMSG(("BS_DIBPATTERN8X8"));
		break;
	case BS_MONOPATTERN:
		DBGMSG(("BS_MONOPATTERN"));
		break;
	}

	DBGMSG(("\n"));
	DBGMSG(("\tLOGBRUSH::lbColor = 0x%x\n", (int)plb->lbColor));
	DBGMSG(("\tLOGBRUSH::lbHatch = "));

	if (plb->lbStyle == BS_HATCHED) {
		switch (plb->lbHatch) {
		case HS_HORIZONTAL:
			DBGMSG(("HS_HORIZONTAL"));
			break;
		case HS_VERTICAL:
			DBGMSG(("HS_VERTICAL"));
			break;
		case HS_FDIAGONAL:
			DBGMSG(("HS_FDIAGONAL"));
			break;
		case HS_BDIAGONAL:
			DBGMSG(("HS_BDIAGONAL"));
			break;
		case HS_CROSS:
			DBGMSG(("HS_CROSS"));
			break;
		case HS_DIAGCROSS:
			DBGMSG(("HS_DIAGCROSS"));
			break;
		}
	} else {
		DBGMSG(("0x%x", (int)plb->lbHatch));
	}

	DBGMSG(("\n"));
}

void DUMP_BITMAPINFOHEADER(BITMAPINFOHEADER *pbih)
{
	DBGMSG(("\tsizeof(BITMAPINFOHEADER)          = %d\n", sizeof(BITMAPINFOHEADER)));
	DBGMSG(("\tBITMAPINFOHEADER::biSize          = %d\n", pbih->biSize));
	DBGMSG(("\tBITMAPINFOHEADER::biWidth         = %d\n", pbih->biWidth));
	DBGMSG(("\tBITMAPINFOHEADER::biHeight        = %d\n", pbih->biHeight));
	DBGMSG(("\tBITMAPINFOHEADER::biPlanes        = %d\n", pbih->biPlanes));
	DBGMSG(("\tBITMAPINFOHEADER::biBitCount      = %d\n", pbih->biBitCount));
	DBGMSG(("\tBITMAPINFOHEADER::biCompression   = %d\n", pbih->biCompression));
	DBGMSG(("\tBITMAPINFOHEADER::biSizeImage     = %d\n", pbih->biSizeImage));
	DBGMSG(("\tBITMAPINFOHEADER::biXPelsPerMeter = %d\n", pbih->biXPelsPerMeter));
	DBGMSG(("\tBITMAPINFOHEADER::biYPelsPerMeter = %d\n", pbih->biYPelsPerMeter));
	DBGMSG(("\tBITMAPINFOHEADER::biClrUsed       = %d\n", pbih->biClrUsed));
	DBGMSG(("\tBITMAPINFOHEADER::biClrImportant  = %d\n", pbih->biClrImportant));
};

void DUMP_XFORM(XFORM *pxform)
{
	DBGMSG(("\teM11 = %f\n", pxform->eM11));
	DBGMSG(("\teM12 = %f\n", pxform->eM12));
	DBGMSG(("\teM21 = %f\n", pxform->eM21));
	DBGMSG(("\teM22 = %f\n", pxform->eM22));
	DBGMSG(("\teDx  = %f\n", pxform->eDx));
	DBGMSG(("\teDy  = %f\n", pxform->eDy));
}

#endif	/* DBG */
