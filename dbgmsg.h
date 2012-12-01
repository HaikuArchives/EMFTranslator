/*
 * dbgmsg.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#ifndef __DBGMSG_H
#define __DBGMSG_H

//#define DBG

#ifdef DBG
	void write_debug_stream(const char *, ...);
	#define DBGMSG(args)	write_debug_stream args

	class  BMessage;
	class  BDirectory;
	struct ENHMETAHEADER;
	struct EMRTEXT;
	struct LOGFONTW;
	struct EXTLOGFONTW;
	struct LOGPEN;
	struct LOGBRUSH;
	struct BITMAPINFOHEADER;
	struct XFORM;

	void DUMP_BFILE(BFile *file, const char *name);
	void DUMP_BMESSAGE(BMessage *msg);
	void DUMP_BDIRECTORY(BDirectory *dir);
	void DUMP_ENHMETAHEADER(ENHMETAHEADER *);
	void DUMP_EMRTEXT(EMRTEXT *);
	void DUMP_LOGFONT(LOGFONTW *);
	void DUMP_EXTLOGFONT(EXTLOGFONTW *);
	void DUMP_LOGPEN(LOGPEN *);
	void DUMP_LOGBRUSH(LOGBRUSH *);
	void DUMP_BITMAPINFOHEADER(BITMAPINFOHEADER *);
	void DUMP_XFORM(XFORM *);
#else
	#define DUMP_BFILE(file, name)		(void)0
	#define DUMP_BMESSAGE(msg)			(void)0
	#define DUMP_BDIRECTORY(dir)		(void)0
	#define DUMP_ENHMETAHEADER(pemh)	(void)0
	#define DUMP_EMRTEXT(pemr)			(void)0
	#define DUMP_LOGFONT(pemr)			(void)0
	#define DUMP_EXTLOGFONT(pemr)		(void)0
	#define DUMP_LOGPEN(pemr)			(void)0
	#define DUMP_LOGBRUSH(pemr)			(void)0
	#define DUMP_BITMAPINFOHEADER(pemr)	(void)0
	#define DUMP_XFORM(pxform)			(void)0
	#define DBGMSG(args)				(void)0
#endif

#endif	/* __DBGMSG_H */
