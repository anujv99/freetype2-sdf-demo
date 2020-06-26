
#ifndef _DEMO_H_
#define _DEMO_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "texture.h"

class demo {
public:
	static void init();
	static void update();
	static void gui();
	static void destroy();
private:
	static void update_glyph();
private:
	static FT_Library library;
	static FT_Face face;
	static texture * default_tex;
	static texture * sdf_tex;

	static int glyph_index;
	static int pixel_size;
	static int spread;
};

#endif //_DEMO_H_
