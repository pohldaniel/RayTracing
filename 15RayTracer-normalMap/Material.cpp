#include "Material.h"
#include "Scene.h"
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

////////////////////////////////////////////////////Phong//////////////////////////////////////////////////////
Phong::Phong(): Material(){

}

Phong::Phong(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies): Material(ambient, diffuse, specular, shinies){

}


Phong::~Phong(){}

float Phong::calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	

	float diff = Vector3f::dot(w_i, hit.normal);
	
	return max(0.0, diff);
}

float Phong::calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	Vector3f V = w_i - (hit.normal  * 2.0f *Vector3f::dot(w_i, hit.normal));

	float spec = Vector3f::dot(w_0, V);


	if (spec > 0.0) return   powf(spec, m_shinies);

	return 0.0;
	
}

Color Phong::shade(const Hit &hit, const Vector3f &w_0){

	Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
	
	for (unsigned int i = 0; i < hit.m_scene->m_lights.size(); i++){

		Vector3f L = (hit.m_scene->m_lights[i]->m_position - hit.hitPoint).normalize(); // Lightdirection | w_i

		// I_in * k_ambiente
		ambiente = ambiente + hit.m_scene->m_lights[i]->m_ambient;

		// I_in * k_diffuse * (L * N)
		diffuse = diffuse + (hit.m_scene->m_lights[i]->m_diffuse * calcDiffuse(hit, w_0, L));
		
		// I_in * k_specular * (R * V)^20
		specular = specular + (hit.m_scene->m_lights[i]->m_specular * calcSpecular(hit, w_0, L));
		
		//std::cout << calcSpecular(hit, w_0, L) << "++++++++" <<std::endl;
	}

	ambiente = ambiente * m_ambient;
	diffuse = diffuse * m_diffuse;
	specular = specular * m_specular;

	

	//std::cout << (diffuse).r << "  " << (diffuse).g << "  " << (diffuse).b << std::endl;

	return ambiente + diffuse + specular;	
}