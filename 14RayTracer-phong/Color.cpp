#include "Color.h"

Color::~Color(){}

Color::Color(){
	Color::r = 1.0;
	Color::g = 1.0;
	Color::b = 1.0;
}



Color::Color(float r, float g, float b){

	Color::r = r;
	Color::g = g;
	Color::b = b;

}

Color::Color(const Color &c){

	r = c.r;
	g = c.g;
	b = c.b;
}

void Color::clamp(){
	r = (r > 1.0) ? 1.0 : (r < 0) ? 0 : r;
	g = (g > 1.0) ? 1.0 : (g < 0) ? 0 : g;
	b = (b > 1.0) ? 1.0 : (b < 0) ? 0 : b;
}


Color &Color::operator+(const Color &rhs)const {

	return Color(this->r + rhs.r, this->g + rhs.g, this->b + rhs.b);
}

Color &Color::operator*(const Color &rhs)const {

	return Color(this->r * rhs.r, this->g * rhs.g, this->b * rhs.b);
}

Color Color::operator*(float scalar) const{

	return Color(r * scalar, g * scalar, b * scalar);
}

Color Color::operator/(float scalar) const{

	return Color(r / scalar, g / scalar, b / scalar);
}