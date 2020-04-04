#include <algorithm>
#include "Color.h"

Color::~Color(){}

Color::Color(){
	r = 1.0;
	g = 1.0;
	b = 1.0;
}



Color::Color(const float a_r, const float a_g, const float a_b){

	r = a_r;
	g = a_g;
	b = a_b;

}



void Color::clamp(){
	r = (r > 1.0f) ? 1.0f : (r < 0.0f) ? 0.0f : r;
	g = (g > 1.0f) ? 1.0f : (g < 0.0f) ? 0.0f : g;
	b = (b > 1.0f) ? 1.0f : (b < 0.0f) ? 0.0f : b;
}


Color Color::operator+(const Color &rhs)const {


	return Color(this->r + rhs.r, this->g + rhs.g, this->b + rhs.b);
}

Color Color::operator*(const Color &rhs)const {

	return Color(this->r * rhs.r, this->g * rhs.g, this->b * rhs.b);
}

Color Color::operator*(float scalar) const{

	return Color(r * scalar, g * scalar, b * scalar);
}

Color Color::operator/(float scalar) const{

	return Color(r / scalar, g / scalar, b / scalar);
}

float Color::Max(){

	return std::max(r, std::max(g, b));
}

Color Color::getRandom() {
	
	return Color((double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX);
}