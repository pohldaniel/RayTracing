#include "Material.h"
#include "Scene.h"
Material::Material(){

	m_shinies = 20;
	m_ambient = Color(0.1, 0.1, 0.1);
	m_diffuse = Color(0.8, 0.8, 0.8);
	m_specular = Color(0.6, 0.6, 0.6);

	colorMapPath = "";
	bumpMapPath = "";
}

Material::Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies){

	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_shinies = shinies;

	colorMapPath = "";
	bumpMapPath = "";
}

Material::Material(const std::shared_ptr<Material> material) : m_ambient(material->m_ambient),
												m_diffuse(material->m_diffuse),
												m_specular(material->m_specular),
												m_shinies(material->m_shinies),
												colorMapPath(material->colorMapPath),
												bumpMapPath(material->bumpMapPath)
{}


Material::~Material(){}

void Material::setAmbient(const Color &ambient){
	m_ambient = ambient;
}
void Material::setDiffuse(const Color &diffuse){
	m_diffuse = diffuse;
}
void Material::setSpecular(const Color &specular){
	m_specular = specular;
}
void Material::setshinies(const int shinies){
	m_shinies = shinies;
}


////////////////////////////////////////////////////Phong//////////////////////////////////////////////////////
Phong::Phong(): Material(){

}

Phong::Phong(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies): Material(ambient, diffuse, specular, shinies){

}

Phong::Phong(const std::shared_ptr<Material> material) : Material(material){


}

Phong::~Phong(){}

float Phong::calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	float diff = Vector3f::dot(w_i, hit.normal);

	return max(0.0, diff);
}

float Phong::calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	Vector3f V = w_i - (hit.normal  * 2.0f *Vector3f::dot(w_i, hit.normal));

	float spec = Vector3f::dot(w_0, V);

	return powf(max(spec, 0.0f), m_shinies);

}

Color Phong::shade(Hit &hit, const Vector3f &w_0){

	Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
	
	for (unsigned int i = 0; i < hit.m_scene->m_lights.size(); i++){

		Vector3f L = (  hit.m_scene->m_lights[i]->m_position - hit.hitPoint).normalize(); // Lightdirection | w_i
		//Vector3f L = ((Vector4f(hit.m_scene->m_lights[i]->m_position, 1.0)*hit.modelView ) - hit.hitPoint).normalize();
		// I_in * k_ambiente
		ambiente = ambiente + hit.m_scene->m_lights[i]->m_ambient;

		// I_in * k_diffuse * (L * N)
		diffuse = diffuse + (hit.m_scene->m_lights[i]->m_diffuse * calcDiffuse(hit, w_0, L));
		
		// I_in * k_specular * (R * V)^20
		specular = specular + (hit.m_scene->m_lights[i]->m_specular * calcSpecular(hit, w_0, L));
		
	}

	ambiente = ambiente * m_ambient;
	diffuse = diffuse * m_diffuse;
	specular = specular * m_specular;

	return hit.color *(ambiente + diffuse + specular);
}

///////////////////////////////////////////NormalMap//////////////////////////////////////////////////

NormalMap::NormalMap() : Material(){

}

NormalMap::NormalMap(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies) : Material(ambient, diffuse, specular, shinies){

}

NormalMap::NormalMap(const std::shared_ptr<Material> material) : Material(material){

}
NormalMap::~NormalMap(){}

void NormalMap::setNormalMap(std::unique_ptr<Texture> normalMap){
	
	m_normalMap = std::move(normalMap);
	
}

float NormalMap::calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	float diff = Vector3f::dot(w_i, hit.normal);

	return max(0.0, diff);
}

float NormalMap::calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i){

	Vector3f V = w_i - (hit.normal  * 2.0f *Vector3f::dot(w_i, hit.normal)) ;

	float spec = Vector3f::dot(w_0, V);

	return powf(max(spec, 0.0f), m_shinies);

}

Color NormalMap::shade( Hit &hit, const Vector3f &a_w_0){

	Matrix4f TBN = getTBN(hit);

	Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
	Vector3f w_0 = (TBN *a_w_0).normalize();
	
	Color tmp = m_normalMap->getTexel(hit.u, hit.v);

	hit.normal = ( (Vector3f(tmp.r, tmp.g, tmp.b) * 2.0 - Vector3f(1.0, 1.0, 1.0))).normalize();

	for (unsigned int i = 0; i < hit.m_scene->m_lights.size(); i++){

		//Vector3f L = ((Vector4f(hit.m_scene->m_lights[i]->m_position, 1.0)*hit.modelView) - hit.hitPoint).normalize(); // Lightdirection | w_i
		Vector3f L = (hit.m_scene->m_lights[i]->m_position - hit.hitPoint).normalize();

		L = (TBN * L).normalize();
		
		// I_in * k_ambiente
		ambiente = ambiente + hit.m_scene->m_lights[i]->m_ambient;

		float lambert = Vector3f::dot(L, hit.normal);
		
		if (lambert > 0.0){

			diffuse = diffuse + (hit.m_scene->m_lights[i]->m_diffuse * lambert);

			Vector3f V = L - (hit.normal  * 2.0f *Vector3f::dot(L, hit.normal)) ;
			specular = specular + Color((pow(max(Vector3f::dot(w_0, V), 0.0), m_shinies)), (pow(max(Vector3f::dot(w_0, V), 0.0), m_shinies)), (pow(max(Vector3f::dot(w_0, V), 0.0), m_shinies)));
		}

		

		// I_in * k_diffuse * (L * N)
		//diffuse = diffuse + (hit.m_scene->m_lights[i]->m_diffuse * calcDiffuse(hit, w_0, L));

		// I_in * k_specular * (R * V)^20
		//specular = specular + (hit.m_scene->m_lights[i]->m_specular * calcSpecular(hit, w_0, L));

	}

	ambiente = ambiente * m_ambient;
	diffuse = diffuse * m_diffuse;
	specular = specular * m_specular ;


	return hit.color *(ambiente + diffuse + specular);
}

Matrix4f NormalMap::getTBN(const Hit &hit){

	Matrix4f tmp1 = Matrix4f(hit.tangent[0], hit.tangent[1], hit.tangent[2], 0.0f,
		hit.bitangent[0], hit.bitangent[1], hit.bitangent[2], 0.0f,
		hit.normal[0], hit.normal[1], hit.normal[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	Matrix4f tmp2 = Matrix4f(hit.tangent[0], hit.bitangent[0], hit.normal[0], 0.0f,
		hit.tangent[1], hit.bitangent[1], hit.normal[1], 0.0f,
		hit.tangent[2], hit.bitangent[2], hit.normal[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	/*Matrix4f tmp =  tmp2;
	std::cout << tmp[0][0] << "  " << tmp[0][1] << "  " << tmp[0][2] << " " << tmp[0][3] << std::endl;
	std::cout << tmp[1][0] << "  " << tmp[1][1] << "  " << tmp[1][2] << " " << tmp[1][3] << std::endl;
	std::cout << tmp[2][0] << "  " << tmp[2][1] << "  " << tmp[2][2] << " " << tmp[2][3] << std::endl;
	std::cout << tmp[3][0] << "  " << tmp[3][1] << "  " << tmp[3][2] << " " << tmp[3][3] << std::endl;*/


	return tmp1;
}