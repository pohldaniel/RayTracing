#include "Color.h"

Color::~Color(){}

Color::Color(){
	Color::r = 255;
	Color::g = 255;
	Color::b = 255;
}




Color::Color(int r, int g, int b)
{
	Color::r = r;
	Color::g = g;
	Color::b = b;
}