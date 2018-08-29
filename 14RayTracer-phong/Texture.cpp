#include <windows.h>
#include <atlstr.h> 
#include <iostream>
#include "Texture.h"

Texture::Texture(const char* path){
	bitmap = std::unique_ptr<Bitmap>(new Bitmap());
	bitmap->loadBitmap24(path);
	width = bitmap->width;
	height = bitmap->height;
	padWidth = bitmap->padWidth;
	Texture::uscale = 1.0;
	Texture::vscale = 1.0;

	
}


Texture::~Texture(){

}

void Texture::setUVScale(const float a_uscale, const float a_vscale){
	uscale = a_uscale;
	vscale = a_vscale;
}

Color Texture::getTexel(const float a_u, const float a_v){

	int u = (((int)(a_u*uscale*(width - 1)))  ) % width;
	int v = (((int)(a_v*vscale*(height - 1))) ) % height;

	//std::cout << width << "  " << height << std::endl;
	
	//std::cout << a_u << "  " << a_v << std::endl;
	//std::cout << u << "  " << v << std::endl;

	int r = bitmap->data[padWidth*v + 3*u];
	int g = bitmap->data[padWidth*v + 3 * u  +1];
	int b = bitmap->data[padWidth*v + 3 * u + 2];
	
	//std::cout << r << "  " << g << "  " << b << std::endl;

	return Color(r / 255.0, g / 255.0, b / 255.0) ;

	//return Color(1, 0, 0);
}
