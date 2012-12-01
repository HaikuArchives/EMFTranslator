/*
 * config.h
 * Copyright 1999 Y.Takagi. All Rights Reserved.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

class emf_settings {
public:
	emf_settings();
	~emf_settings();
	void SetSettings(int w, int h);
	void GetSettings(int *w, int *h) const;
private:
	int  max_width;
	int  max_height;
	bool settings_touched;
};

extern emf_settings g_settings;

#endif	// __CONFIG_H
