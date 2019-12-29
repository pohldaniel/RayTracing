#include "Material.h"
#include "Scene.h"

Material::Material(){
	
	m_shinies = 20;
	m_ambient = Color(1.0, 1.0, 1.0);
	m_diffuse = Color(1.0, 1.0, 1.0);
	m_specular = Color(1.0, 1.0, 1.0);
	m_normalMap = NULL;

	colorMapPath = "";
	bumpMapPath = "";
}

Material::Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies = 20){

	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
	m_shinies = shinies;
	m_normalMap = NULL;

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

void Material::setAmbient(float ambient){

	m_ambient = Color(ambient, ambient, ambient);
}

void Material::setDiffuse(const Color &diffuse){
	m_diffuse = diffuse;

}

void Material::setDiffuse(float diffuse){

	m_diffuse = Color(diffuse, diffuse, diffuse);
}

void Material::setSpecular(const Color &specular){

	m_specular = specular;
}

void Material::setSpecular(float specular){

	m_specular = Color(specular, specular, specular);
}

void Material::setColor(const Color &color){
	m_diffuse = color;
	m_ambient = color;
	m_specular = color;
}

void Material::setColor(float color){
	m_diffuse = Color(color, color, color);
	m_ambient = Color(color, color, color);
	m_specular = Color(color, color, color);
}

void Material::setShinies(const int shinies){
	m_shinies = shinies;
}

void Material::setNormalTexture(ImageTexture* normalMap){

	m_normalMap = std::shared_ptr<ImageTexture>(normalMap);
}

Matrix4f Material::getTBN(const Hit &hit){

	return Matrix4f(hit.tangent[0], hit.tangent[1], hit.tangent[2], 0.0f,
		hit.bitangent[0], hit.bitangent[1], hit.bitangent[2], 0.0f,
		hit.normal[0], hit.normal[1], hit.normal[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

Vector3f Material::Bump(Hit &hit){

	if (m_normalMap){

		
	}

	return Vector3f(0.0, 0.0, 0.0);
}

Color Material::shadeAreaLight(Hit &hit) {
	return Color(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////Phong//////////////////////////////////////////////////////
Phong::Phong(): Material(){}

Phong::Phong(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies): Material(ambient, diffuse, specular, shinies){}

Phong::Phong(const std::shared_ptr<Material> material) : Material(material){}

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

Color Phong::shade(Hit &hit){
	
	Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
	
	// -w_0
	Vector3f incidentRay = hit.transformedRay.direction;

	if (m_normalMap){

		Matrix4f TBN = getTBN(hit);
		Color tmp = m_normalMap->getTexel(hit.u, hit.v, Vector3f(0.0, 0.0, 0.0));
		//push back the normal of the normalMap to the object Space with the inverse(transpose) TBN
		hit.normal = (TBN* (Vector3f(tmp.r, tmp.g, tmp.b) * 2.0 - Vector3f(1.0, 1.0, 1.0))).normalize();
	}

	for (unsigned int i = 0; i < hit.scene->m_lights.size(); i++){

		//w_i | hitpoint to lightpos | Lightdirection
		Vector3f L = (hit.scene->m_lights[i]->m_position - hit.hitPoint).normalize();

		// I_in * k_ambiente
		ambiente = ambiente + hit.scene->m_lights[i]->m_ambient;

		float lambert = Vector3f::dot(L, hit.normal);

		if (lambert > 0.0){

			//diffuse = diffuse + (hit.m_scene->m_lights[i]->m_diffuse * calcDiffuse(hit, incidentRay, L));
			diffuse = diffuse + (hit.scene->m_lights[i]->m_diffuse * lambert);

			Vector3f reflectedRay = L - (hit.normal  * 2.0f *Vector3f::dot(L, hit.normal));
			//specular + (hit.m_scene->m_lights[i]->m_specular * calcSpecular(hit, incidentRay, L));
			specular = specular + (hit.scene->m_lights[i]->m_specular * pow(max(Vector3f::dot(incidentRay, reflectedRay), 0.0), m_shinies));
		}
	}

	ambiente = ambiente * m_ambient;
	diffuse = diffuse * m_diffuse;
	specular = specular * m_specular ;

	Color color = hit.color *(ambiente + diffuse + specular);

	return color;
}

////////////////////////////////////////////////////Matte///////////////////////////////////////////////////////////
Matte::Matte() : Material(){
	m_ka = 1.0;
	m_kd = 1.0;
}

Matte::Matte(const Color &ambient, const Color &diffuse, const Color &specular) : Material(ambient, diffuse, specular){
	m_ka = 1.0;
	m_kd = 1.0;
}

Matte::Matte(const std::shared_ptr<Material> material) : Material(material){
	m_ka = 1.0;
	m_kd = 1.0;
}

Matte::~Matte(){}

void Matte::setKd(const float kd){

	m_kd = kd;
}

void Matte::setKa(const float ka){

	m_ka = ka;
}

Color Matte::shade(Hit &hit){

	Color L = Color(0.0, 0.0, 0.0);

	if (hit.scene->m_ambient){

		L = (hit.color* m_ka)  * hit.scene->m_ambient->L(hit);
	}

	for (unsigned int i = 0; i < hit.scene->m_lights.size(); i++){
				
		Vector3f wi = hit.scene->m_lights[i]->getDirection(hit.hitPoint);

		float lambert = Vector3f::dot( hit.normal, wi);

		if (lambert > 0.0){
			
			if (hit.scene->m_lights[i]->m_castShadow){
				
				Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
				Ray	_ray = Ray(transformedhitPoint + hit.normal * 0.01, wi);
				bool hitObject = false;

				for (unsigned int j = 0; j < hit.scene->m_primitives.size(); j++){
	
					hitObject = hitObject || hit.scene->m_primitives[j]->shadowHit(_ray);
					if (hitObject) break;
				}

				if (!hitObject){
					L = L + (hit.color* m_kd * invPI) * hit.scene->m_lights[i]->L(hit) * lambert;
				}

			}else{
				
					L = L + (hit.color* m_kd * invPI) * hit.scene->m_lights[i]->L(hit) * lambert;
			}
		}
	}
	
	return L;
}

Color Matte::shadeAreaLight(Hit &hit) {

	Color L = Color(0.0, 0.0, 0.0);

	if (hit.scene->m_ambient){
		//ambient_brdf->rho(sr, wo) * sr.w.ambient_ptr->L(sr);
		L = (m_ambient * m_ka)  * hit.scene->m_ambient->L(hit);
	}

	for (unsigned int i = 0; i < hit.scene->m_lights.size(); i++) {

		AreaLight* light = static_cast<AreaLight*>(hit.scene->m_lights[0].get());
		Vector3f wi = light->getDirection(hit.hitPoint);
		float lambert = Vector3f::dot(hit.normal, wi);

		if (lambert > 0.0){
		
			if (hit.scene->m_lights[i]->m_castShadow){
			
				Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
				Ray	_ray = Ray(transformedhitPoint + hit.normal * 0.01, wi);
				bool hitObject = false;

				for (unsigned int j = 0; j < hit.scene->m_primitives.size(); j++){

					if (light->m_primitive == hit.scene->m_primitives[j]) continue;

					hitObject = hitObject || hit.scene->m_primitives[j]->shadowHit(_ray);
					if (hitObject) break;
				}

				if (!hitObject){
				
					L = L + ((hit.color * invPI * m_kd * light->L(hit) * light->G(hit) * lambert) / light->pdf(hit));
				}
			}else{
				L = L + ((hit.color * invPI * m_kd * light->L(hit) * light->G(hit) * lambert) / light->pdf(hit));
			}
		}
	}
	
	return L;
}

////////////////////////////////////////////////////Reflective//////////////////////////////////////////////////////
Reflective::Reflective( ) : Material(){

	m_reflectionColor = Color(1.0, 1.0, 1.0);
	m_frensel = 0.5;

	m_phong = std::shared_ptr<Phong>(new Phong());
}

Reflective::Reflective(const Color &ambient, const Color &diffuse, const Color &specular) : Material(ambient, diffuse, specular){

	m_reflectionColor = Color(1.0, 1.0, 1.0);
	m_frensel = 0.5;

	m_phong = std::shared_ptr<Phong>(new Phong());
}

Reflective::Reflective(const std::shared_ptr<Material> material) : Material(material){

	m_reflectionColor = Color(1.0, 1.0, 1.0);
	m_frensel = 0.5;

	m_phong = std::shared_ptr<Phong>(new Phong());
}

Reflective::~Reflective(){}


void Reflective::setReflectionColor(const Color &reflectionColor){

	m_reflectionColor = reflectionColor;
}

void Reflective::setReflectionColor(float reflectionColor){

	m_reflectionColor = Color(reflectionColor, reflectionColor, reflectionColor);
}

void Reflective::setFrensel(float frensel){

	m_frensel = frensel;
}

Color Reflective::shade(Hit &hit){

	
	if (m_normalMap){

		Matrix4f TBN = getTBN(hit);
		Color tmp = m_normalMap->getSmoothTexel(hit.u, hit.v);
		//push back the normal of the normalMap to the object Space with the inverses TBN
		hit.normal = (TBN * (Vector3f(tmp.r, tmp.g, tmp.b) * 2.0 - Vector3f(1.0, 1.0, 1.0))).normalize();
	}

	hit.color = m_phong->shade(hit);

	//Ray from the eye to the transformed hitpoint Tp
	//-w_0
	Vector3f incidentRay = hit.originalRay.direction;

	// to get the reflected ray in eyespace use the transformed normal
	// w_i
	float dot = Vector3f::dot(incidentRay, hit.normal);
	Vector3f reflectedRay = incidentRay - (hit.normal  * 2.0f * dot);

	//transformed hitpoint Tp  (T^-1o + T^-1td =p => o + td = Tp)
	// this point is in  eyespace 
	Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t ;

	hit.originalRay.origin = dot < 0 ? transformedhitPoint + hit.normal * 0.01 : transformedhitPoint - hit.normal * 0.01;
	hit.originalRay.direction = reflectedRay;
	

	
	// reflection Color * frenselterm * weight 
	Color brdf = m_reflectionColor * m_frensel * (1 / Vector3f::dot(incidentRay, hit.normal)) ;
	
	// cast an additional ray in eyespace, so the transformation at hitobjects can done as usually
	return hit.color + brdf * hit.scene->traceRay(hit.originalRay) * Vector3f::dot(incidentRay, hit.normal);	
}
////////////////////////////////////////////////////Emissive//////////////////////////////////////////////////////
Emissive::Emissive() : Material(), m_ls(1.0) { }

Emissive::Emissive(const Color &ambient, const Color &diffuse, const Color &specular) : Material(ambient, diffuse, specular), m_ls(1.0){}

Emissive::Emissive(const std::shared_ptr<Material> material) : Material(material){}

Emissive::~Emissive(){}

void Emissive::setScaleRadiance(const float ls){

	m_ls = ls;
}

Color Emissive::getLe(Hit &hit) const{

	float dot = Vector3f::dot(hit.originalRay.direction, hit.normal);

	if (dot <= 0.0){
		return m_ambient * m_ls;

	}else{
		return Color(0.0, 0.0, 0.0);
	}
}

Color Emissive::shade(Hit &hit){

	float dot = Vector3f::dot(hit.originalRay.direction, hit.normal);

	if (dot <= 0.0){
		return hit.color * m_ls;

	}else{
		return Color(0.0, 0.0, 0.0);
	}
}

Color Emissive::shadeAreaLight(Hit &hit){

	float dot = Vector3f::dot(hit.originalRay.direction, hit.normal);

	if (dot <= 0.0){
		return m_diffuse * m_ls;

	}else{
		return Color(0.0, 0.0, 0.0);
	}

}