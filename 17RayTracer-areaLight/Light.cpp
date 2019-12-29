#include <iostream>
#include "Light.h"

Light::Light(){

	m_position = Vector3f(0.0, 0.0, 30);
	m_ambient = Color(1.0, 1.0, 1.0);
	m_diffuse = Color(1.0, 1.0, 1.0);
	m_specular = Color(1.0, 1.0, 1.0);
	m_castShadow = false;
}

Light::Light(const Vector3f &a_position, const Color &ambiente, const Color &diffuse, const Color &specular){
	
	m_position = a_position;
	m_ambient = ambiente;
	m_diffuse = diffuse;
	m_specular = specular;
	m_castShadow = false;
}

Vector3f Light::getDirection(const Vector3f &hitPoint){

	return Vector3f(0.0, 0.0, 0.0);
}

Color Light::L(Hit &hit){

	return Color(0.0, 0.0, 0.0);
}

void Light::setShadows(bool castShadow){
	
	m_castShadow = castShadow;
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

Color AmbientLight::L(Hit &hit){

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

Color DirectionalLight::L(Hit &hit){

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

Color PointLight::L(Hit &hit){

	return (m_ambient * m_ls);
}
////////////////////////////////////////////////////AreaLight///////////////////////////////////////////////////////////
AreaLight::AreaLight() {
	m_ambient = Color(1.0, 1.0, 1.0);
	m_diffuse = Color(1.0, 1.0, 1.0);
	m_specular = Color(1.0, 1.0, 1.0);
}

AreaLight::AreaLight(const Color &ambiente, const Color &diffuse, const Color &specular) {		 
	m_ambient = ambiente;
	m_diffuse = diffuse;	
	m_specular = specular;

}

AreaLight::AreaLight(const Color &color){
	m_ambient = color;
	m_diffuse = color;
	m_specular = color;
}

AreaLight::~AreaLight(){}

void AreaLight::setObject(Primitive* primitive){
	m_primitive = std::shared_ptr<Primitive>(primitive);	m_material = primitive->getMaterial();
}

Vector3f AreaLight::getDirection(const Vector3f &hitPoint) {
	m_samplePoint = m_primitive->sample();					// used in the G function
	m_lightNormal = m_primitive->getNormal(m_samplePoint);	// used in the G and L function
	m_wi = (m_samplePoint -hitPoint ).normalize(); 			// used in the G and L function

	return m_wi;
}

Color AreaLight::L(Hit &hit) {

	if (Vector3f::dot(m_lightNormal, m_wi) <= 0.0){
		return static_cast<Emissive*>(m_material.get())->getLe(hit);
	}else{

		return Color(0.0, 0.0, 0.0);
	}
}

float AreaLight::G(Hit &hit) const {
	
	float lambda1 = -Vector3f::dot(m_lightNormal, m_wi);
	Vector3f diff = hit.hitPoint - m_samplePoint;
	float d2 = Vector3f::dot(diff, diff);
	return (lambda1 / d2);
}

float AreaLight::pdf(Hit &hit) const {
	return (m_primitive->pdf(hit));
	
}

