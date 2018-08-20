#include <iostream>
#include "Light.h"

Light::Light(Vector3f position, Color color){
	Light::position = position;
	Light::color = color;
}

Light::~Light(){}

const Color &Light::getColor() const{
	return color;
}

float Light::calcDiffuse(Vector3f& a_Pos, Vector3f& a_Normal){

	Vector3f L = (position - a_Pos).normalize();
	float diff = Vector3f::dot(L, a_Normal) ;
	
	if (diff > 0){

		return diff;

	}else {

		return 0.0;
	}


}

float Light::calcSpecular(Vector3f& a_Pos, Vector3f& a_Normal, Vector3f& a_viewDirection, int a_n){

	Vector3f L = (position - a_Pos).normalize();
	Vector3f V = L - (a_Normal  * 2.0f *Vector3f::dot(L, a_Normal)) ;

	

	float spec = Vector3f::dot(a_viewDirection, V);

	if (spec > 0.0){

		return   powf(spec, a_n);

	} else {

		return 0.0;

	}

	
}