#include <iostream>

#include "scene.h"

Scene::Scene() {

	m_vp = ViewPlane();
	m_background = Color(0.0, 0.0, 0.0);
	m_bitmap = NULL;
	m_scene = std::shared_ptr<Scene>(this);
	m_maximumDepth = 0;
	m_ambient = std::unique_ptr<AmbientLight>(new AmbientLight());

	

	try {

		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(m_vp.vres, m_vp.hres, 24));

	}catch (const char* e) {

		std::cout << "Could not load Scene bitmap!" << std::endl;
	}

	
}

Scene::Scene(const ViewPlane &vp, const Color &background){

	m_vp = vp;
	m_background = background;
	m_bitmap = NULL;
	m_scene = std::shared_ptr<Scene>(this);
	m_maximumDepth = 0;



	try {

		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(vp.vres, vp.hres, 24));

	}catch (const char* e) {
		std::cout << "Could not load Scene bitmap!" << std::endl;

	}


}


void Scene::addPrimitive(Primitive* primitive) {
	m_primitives.push_back(std::shared_ptr<Primitive>(primitive));
}

void Scene::addLight(Light* light) {
	m_lights.push_back(std::unique_ptr<Light>(light));
}

void Scene::setAmbientLight(AmbientLight *ambient){

	m_ambient = std::unique_ptr<AmbientLight>(ambient);
}

void Scene::setPixel(const int x, const int y, Color& color)const {
	color.clamp();
	int r = (int)(color.r * 255.0);
	int g = (int)(color.g * 255.0);
	int b = (int)(color.b * 255.0);


	m_bitmap->setPixel24(x, y, r, g, b);
}


void Scene::setDepth(int depth){

	m_maximumDepth = depth;
}

std::shared_ptr<Bitmap> Scene::getBitmap(){

	return m_bitmap;
}

ViewPlane Scene::getViewPlane(){

	return m_vp;
}

Hit Scene::hitObjects(Ray& _ray)  {
	
	float	 tmin = FLT_MAX;
	Hit		 hit;
	
	
	hit.color = m_background;
	hit.scene = m_scene;
	hit.originalRay = _ray;
	m_primitive = NULL;
	//after the for loop the hit.transformedRay is transformed to next local space of the primitive
	//to avoid transforming to another space from a following primitive
	//it will be necessary to store the transformation
	Ray ray;

	bool hitObject = false;

	

	for (unsigned int j = 0; j < m_primitives.size(); j++){
		hit.transformedRay = _ray;

		m_primitives[j]->hit(hit);

		if (hit.hitObject && hit.t < tmin) {

			tmin = hit.t;
			m_primitive = m_primitives[j];
			ray = hit.transformedRay;
			hitObject = true;
			
		}


	}

	if (hitObject){

			hit.t = tmin;
			hit.hitPoint = ray.origin + ray.direction * tmin;
			hit.color = m_primitive->getColor(hit.hitPoint);
			hit.normal = m_primitive->getNormal(hit.hitPoint);
			hit.tangent = m_primitive->getTangent(hit.hitPoint);
			hit.bitangent = m_primitive->getBiTangent(hit.hitPoint);
			//needed for normal mapping and texturing traiangle meshes
			std::pair <float, float> uv = m_primitive->getUV(hit.hitPoint);
			hit.u = uv.first;
			hit.v = uv.second;
			
			if (m_primitive->getMaterial()){
				
				if (m_primitive->getMaterial()->m_reflective){
	
					hit.color =  m_primitive->getMaterial()->shade(hit);
					
				}else{
					
					hit.color = m_primitive->getMaterial()->shade(hit);
				}

			}else{
				
				hit.color = m_primitive->getColor(hit.hitPoint);
				
			}
	}
	
	return hit;
}

Color Scene::traceRay(Ray& ray){
	
	ray.depth++;
	//std::cout << ray.depth << std::endl;
	if ((ray.depth) == m_maximumDepth + 1){

		return    Color(0.0, 0.0, 0.0);

	}else{

		return hitObjects(ray).color;

	}
}




