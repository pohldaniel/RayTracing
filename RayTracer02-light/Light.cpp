#include "Light.h"

Light::Light(Vector3f position, Color color){
	Light::position = position;
	Light::color = color;
}

Light::~Light(){}

const Color &Light::getColor() const{
	return color;
}