#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <memory>

#include "Bitmap.h"
#include "Color.h"

class Texture
{
public:
	
	Texture();
	Texture(const char* path);
	~Texture();

	Color getTexel(const float u, const float v);
	void setUVScale(const float uscale, const float vscale);

private:

	int m_width, m_height, m_padWidth;
	std::unique_ptr<Bitmap> m_bitmap;
	float m_uscale, m_vscale;
	
};

#endif
