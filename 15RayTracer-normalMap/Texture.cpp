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

	

	//std::cout << width << "  " << height << std::endl;
	
	//std::cout << a_u << "  " << a_v << std::endl;
	//std::cout << u << "  " << v << std::endl;

	int r = m_bitmap->data[m_padWidth*v + 3*u];
	int g = m_bitmap->data[m_padWidth*v + 3 * u  +1];
	int b = m_bitmap->data[m_padWidth*v + 3 * u + 2];
	
	

	return Color(r / 255.0f, g / 255.0f, b / 255.0f) ;

	//return Color(1, 0, 0);
}
