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

Vector3f Light::getDirection(const Vector3f &hitPoint){

	return Vector3f(0.0, 0.0, 0.0);
}

Color Light::L(const Vector3f &pos){

	return Color(0.0, 0.0, 0.0);
}

Light::~Light(){}

////////////////////////////////////////////////////Ambient///////////////////////////////////////////////////////////
AmbientLight::AmbientLight(){

	m_ambient = Color(1.0, 1.0, 1.0);
	m_ls = 1.0;
}

AmbientLight::AmbientLight(const Color &color){

	m_ambient = color;
	m_ls = 1.0;
}

AmbientLight::~AmbientLight(){

}

void AmbientLight::setScaleRadiance(const float radiance){

	m_ls = radiance;
}

Color AmbientLight::L(const Vector3f &pos){

	return (m_ambient * m_ls);
}
////////////////////////////////////////////////////Direction///////////////////////////////////////////////////////////
DirectionalLight::DirectionalLight(){

	m_direction = Vector3f(0.0, 1.0, 0.0);
	m_ambient = Color(1.0, 1.0, 1.0);
	m_diffuse = Color(1.0, 1.0, 1.0);
	m_specular = Color(1.0, 1.0, 1.0);
	m_ls = 1.0;
}

DirectionalLight::DirectionalLight(const Vector3f &direction, const Color &ambiente, const Color &diffuse, const Color &specular){

	m_direction = direction;
	m_ambient = ambiente;
	m_diffuse = diffuse;
	m_specular = specular;
	m_ls = 1.0;
}

DirectionalLight::DirectionalLight(const Vector3f &direction, const Color &color){

	m_ambient = color;
	m_direction = direction;
	m_ls = 1.0;
}

DirectionalLight::~DirectionalLight(){

}

void DirectionalLight::setDirection(const Vector3f &direction){

	m_direction = direction;
}

void DirectionalLight::setScaleRadiance(const float radiance){

	m_ls = radiance;
}

Color DirectionalLight::L(const Vector3f &pos){

	return (m_ambient * m_ls);
}

Vector3f DirectionalLight::getDirection(const Vector3f &hitPoint){

	return m_direction.normalize();
}

////////////////////////////////////////////////////PointLight///////////////////////////////////////////////////////////
PointLight::PointLight(){

	m_position = Vector3f(0.0, 0.0, 0.0);
	m_ambient = Color(1.0, 1.0, 1.0);
	m_diffuse = Color(1.0, 1.0, 1.0);
	m_specular = Color(1.0, 1.0, 1.0);
	m_ls = 1.0;
}

PointLight::PointLight(const Vector3f &pos, const Color &ambiente, const Color &diffuse, const Color &specular){

	m_position = pos;
	m_ambient = ambiente;
	m_diffuse = diffuse;
	m_specular = specular;
	m_ls = 1.0;
}

PointLight::PointLight(const Vector3f &pos, const Color &color){

	m_position = pos;
	m_ambient = color;
	m_diffuse = color;
	m_specular = color;
	m_ls = 1.0;
}

PointLight::~PointLight(){}

void PointLight::setPosition(const Vector3f &pos){

	m_position = pos;	
}

void PointLight::setScaleRadiance(const float radiance){

	m_ls = radiance;
}

Vector3f PointLight::getDirection(const Vector3f &hitPoint){

	return (m_position - hitPoint).normalize();
}

Color PointLight::L(const Vector3f &pos){

	return (m_ambient * m_ls);
}