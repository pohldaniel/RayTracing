#include <iostream>
#include "Light.h"

Light::Light(){

	m_position = Vector3f(0.0, 0.0, 30);
	m_ambient = Color(0.5, 0.5, 0.5);
	m_diffuse = Color(0.5, 0.5, 0.5);
	m_specular = Color(0.5, 0.5, 0.5);
}

Light::Light(const Vector3f &a_position, const Color &ambiente, const Color &diffuse, const Color &specular){
	
	m_position = a_position;
	m_ambient = ambiente;
	m_diffuse = diffuse;
	m_specular = specular;

}

Light::~Light(){}


float Light::calcDiffuse(const Vector3f& a_Pos, const Vector3f& a_Normal){

	Vector3f L = (m_position - a_Pos).normalize();
	float diff = Vector3f::dot(L, a_Normal) ;
	
	if (diff > 0){

		return diff;

	}else {

		return 0.0;
	}


}

float Light::calcSpecular(const Vector3f& a_Pos, const Vector3f& a_Normal, const Vector3f& a_viewDirection, const int a_n){

	Vector3f L = (m_position - a_Pos).normalize();
	Vector3f V = L - (a_Normal  * 2.0f *Vector3f::dot(L, a_Normal)) ;

	

	float spec = Vector3f::dot(a_viewDirection, V);

	if (spec > 0.0){

		return   powf(spec, a_n);

	} else {

		return 0.0;

	}

	
}