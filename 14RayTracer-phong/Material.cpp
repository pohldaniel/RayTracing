#include "Material.h"

Material::Material(){

	Material::m_ambi = 0.0;
	Material::m_diff = 0.0;
	Material::m_spec= 0.0;
}

Material::Material(float a_ambi, float a_diff, float a_spec, int a_n){

	Material::m_ambi = a_ambi;
	Material::m_diff = a_diff;
	Material::m_spec = a_spec;
	Material::m_n	 = a_n;

}

Material::~Material(){}

void Material::setAmbiente(float a_ambi) {
	m_ambi = a_ambi;
}

void Material::setDiffuse(float a_diff) {
	m_diff = a_diff;
}

void Material::setSpecular(float a_spec) {
	m_spec = a_spec;
}

void Material::setSurfaceProperty(float a_n){
	m_n = a_n;
}

float Material::getAmbiente() {
	return m_ambi;
}

float Material::getDiffuse() {

	return m_diff; 
}

float Material::getSpecular() {
	return m_spec;
}

int Material::getSurfaceProperty(){
	return m_n;
}