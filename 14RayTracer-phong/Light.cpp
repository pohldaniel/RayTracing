#include <iostream>
#include "Light.h"



Light::Light(Vector3f a_position, Color *ambiente, Color *diffuse, Color *specular){
	Light::position = a_position;
	
	m_ambient = ambiente;
	m_diffuse = diffuse;
	m_specular = specular;

}

Light::~Light(){}


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