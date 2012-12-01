/*
 * gdicontainer.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#ifndef __GDICONTAINER_H
#define __GDICONTAINER_H

#include <InterfaceDefs.h>
#include "gdi.h"

class GDIObject {
public:
	GDIObject() {}
	~GDIObject() {}
	int id;
};

class GDIPen : public GDIObject {
public:
	GDIPen(const LOGPEN *, const pattern * = NULL);
	~GDIPen() {}
	LOGPEN lp;
	pattern pat;
};

class GDIBrush : public GDIObject {
public:
	GDIBrush(const LOGBRUSH *, const pattern * = NULL);
	~GDIBrush();
	LOGBRUSH lb;
	COLORREF BkColor;
	pattern pat;
};

class GDIFont : public GDIObject {
public:
	GDIFont(const LOGFONTW *);
	~GDIFont() {}
	LOGFONTW lf;
};

class GDIPalette : public GDIObject {
public:
	GDIPalette();
	~GDIPalette() {}
};

class GDIColorSpace : public GDIObject {
public:
	GDIColorSpace();
	~GDIColorSpace() {}
};

class GDIContainer {
public:
	GDIContainer(int n);
	~GDIContainer();
	bool set_at(int index, GDIObject *);
	GDIObject *get_at(int index);

private:
	GDIObject **objects;
	int limit;
};

#endif	// __GDICONTAINER_H
