#include "Color.h"

Color::~Color(){}

Color::Color(){
	Color::r = 255;
	Color::g = 255;
	Color::b = 255;
}



Color::Color(int r, int g, int b)
{
	Color::r = (r > 255) ? 255 : (r < 0) ? 0 : r;
	Color::g = (g > 255) ? 255 : (g < 0) ? 0 : g;
	Color::b = (b > 255) ? 255 : (b < 0) ? 0 : b;

}

Color &Color::operator+(const Color &rhs)const {

	return Color(this->r + rhs.r, this->g + rhs.g, this->b + rhs.b);
}

Color Color::operator*(float scalar) const
{
	return Color(r * scalar, g * scalar, b * scalar);
}