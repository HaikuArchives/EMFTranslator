/*
 * gdicontainer.cpp
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#include <iostream>
#include <memory.h>
#include "gdicontainer.h"

GDIPen::GDIPen(const LOGPEN *plp, const pattern *p) : GDIObject()
{
	memcpy(&lp, plp, sizeof(LOGPEN));
	if (p)
		memcpy(&pat, p, sizeof(pattern));
	id = OBJ_PEN;
}

GDIBrush::GDIBrush(const LOGBRUSH *plb, const pattern *p) : GDIObject()
{
	memcpy(&lb, plb, sizeof(LOGBRUSH));
	if (p)
		memcpy(&pat, p, sizeof(pattern));
	id  = OBJ_BRUSH;
}

GDIBrush::~GDIBrush()
{
}

GDIFont::GDIFont(const LOGFONTW *plf) : GDIObject()
{
	memcpy(&lf, plf, sizeof(LOGFONTW));
	id = OBJ_FONT;
}

GDIPalette::GDIPalette()
{
	id = OBJ_PAL;
}

GDIColorSpace::GDIColorSpace()
{
	id = OBJ_COLORSPACE;
}

GDIContainer::GDIContainer(int n)
{
	objects = new GDIObject*[n];
	memset(objects, 0, sizeof(GDIObject *) * n);
	limit = n;
}

GDIContainer::~GDIContainer()
{
	GDIObject **p = objects;
	while (limit--) {
		if (*p) {
			delete *p;
		}
		p++;
	}
	delete [] objects;
}

bool GDIContainer::set_at(int index, GDIObject *obj)
{
	if (index < limit) {
		GDIObject *o = objects[index];
		if (o) {
			delete o;
		}
		objects[index] = obj;
		return true;
	}
	return false;
}

GDIObject *GDIContainer::get_at(int index)
{
	if (index < limit) {
		return objects[index];
	}
	return NULL;
}
