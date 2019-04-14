#include <windows.h>
#include <atlstr.h> 
#include <iostream>
#include "Texture.h"


Texture::Texture(){
	m_bitmap = std::unique_ptr<Bitmap>(new Bitmap());
	m_bitmap->createNullBitmap(200);

	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;
	m_uscale = 1.0;
	m_vscale = 1.0;
}

Texture::Texture(const char* path){
	m_bitmap = std::unique_ptr<Bitmap>(new Bitmap());

	if (!m_bitmap->loadBitmap24(path)){
		std::cout << "create nulltexture" << std::endl;
		m_bitmap->createNullBitmap(200);
	}


	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;
	m_uscale = 1.0;
	m_vscale = 1.0;
	
}


Texture::~Texture(){

}

void Texture::setUVScale(const float a_uscale, const float a_vscale){
	m_uscale = a_uscale;
	m_vscale = a_vscale;
}

Color Texture::getTexel(const float a_u, const float a_v){

	int u = abs((((int)(a_u*m_uscale*(m_width - 1)))  ) % m_width);
	int v = abs((((int)(a_v*m_vscale*(m_height - 1))) ) % m_height);

	int r = m_bitmap->data[m_padWidth*v + 3*u];
	int g = m_bitmap->data[m_padWidth*v + 3 * u  +1];
	int b = m_bitmap->data[m_padWidth*v + 3 * u + 2];
	
	

	return Color(r / 255.0f, g / 255.0f, b / 255.0f) ;

}

Color Texture::getSmoothTexel(float a_u, float a_v){

	

	float u = m_width  * m_uscale * a_u;
	float v = m_height * m_vscale * a_v;

	int u1 = (m_width + (int)u % m_width) % m_width;
	int v1 = (m_height + (int)v % m_height) % m_height;
	int u2 = (u1 + 1) % m_width;
	int v2 = (v1 + 1) % m_height;


	
	// calculate fractional parts of u and v
	float fracu = u - floorf(u);
	float fracv = v - floorf(v);
	// calculate weight factors
	float w1 = (1 - fracu) * (1 - fracv);
	float w2 = fracu * (1 - fracv);
	float w3 = (1 - fracu) * fracv;
	float w4 = fracu *  fracv;
	// fetch four texels


	Color c1 = Color(m_bitmap->data[m_padWidth*v1 + 3 * u1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u1 + 1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u1 + 2] / 255.0);
	Color c2 = Color(m_bitmap->data[m_padWidth*v1 + 3 * u2] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u2 + 1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u2 + 2] / 255.0);
	Color c3 = Color(m_bitmap->data[m_padWidth*v2 + 3 * u1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u1 + 1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u1 + 2] / 255.0);
	Color c4 = Color(m_bitmap->data[m_padWidth*v2 + 3 * u2] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u2 + 1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u2 + 2] / 255.0);


	// scale and sum the four colors
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}