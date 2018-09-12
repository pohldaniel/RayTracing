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


