#include "Material.h"

Material::Material(){

	m_shinies = 50;
	m_ambient = Color(0.1, 0.1, 0.1);
	m_diffuse = Color(0.8, 0.8, 0.8);
	m_specular = Color(0.6, 0.6, 0.6);

	
}

Material::Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies){

	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_shinies = shinies;
}


Material::~Material(){}

