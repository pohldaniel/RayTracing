#include <iostream>
#include <random>
#include <ctime>
#include "scene.h"

Scene::Scene() {

	m_vp = ViewPlane();
	m_background = Color(0.0, 0.0, 0.0);
	m_bitmap = NULL;
	m_scene = std::shared_ptr<Scene>(this);
	m_maximumDepth = 0;
	m_ambient = std::unique_ptr<AmbientLight>(new AmbientLight());
	m_tracer = Tracer::Whitted;
	
	try {
		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(m_vp.vres, m_vp.hres, 24));
	}catch (const char* e) {
		std::cout << "Could not load Scene bitmap!" << std::endl;
	}	

	std::srand(std::time(NULL));
}

Scene::Scene(const ViewPlane &vp, const Color &background){

	m_vp = vp;
	m_background = background;
	m_bitmap = NULL;
	m_scene = std::shared_ptr<Scene>(this);
	m_maximumDepth = 0;
	m_ambient = std::unique_ptr<AmbientLight>(new AmbientLight());
	m_tracer = Tracer::Whitted;

	try {
		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(vp.vres, vp.hres, 24));
	}catch (const char* e) {
		std::cout << "Could not load Scene bitmap!" << std::endl;
	}

	std::srand(std::time(NULL));
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

	//std::cout << color.r << "  " << color.g << "  " << color.b << std::endl;

	color.clamp();

	/*int r = (int)(pow(color.r, 1 / 2.2) * 255.0 + 0.5);
	int g = (int)(pow(color.g, 1 / 2.2) * 255.0 + 0.5);
	int b = (int)(pow(color.b, 1 / 2.2) * 255.0 + 0.5);*/

	int r = (int)(color.r * 255.0);
	int g = (int)(color.g * 255.0);
	int b = (int)(color.b * 255.0);

	m_bitmap->setPixel24(x, y, r, g, b);
}


void Scene::setDepth(int depth){

	m_maximumDepth = depth;
}

void Scene::setTracer(Tracer tracer){

	m_tracer = tracer;
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

				//to do trigger the funktion through a tracer pointer
				switch (m_tracer) {
				
					case Whitted:
						hit.color = m_primitive->getMaterial()->shade(hit);
						break;
					case AreaLighting:
						hit.color = m_primitive->getMaterial()->shadeAreaLight(hit);
						break;
					case PathTracer:
						//if m_primitive a lightsource the emissive material will return a color != Color(0.0, 0.0, 0.0)
						//and the recursion will break with a color != Color(0.0, 0.0, 0.0)
						hit.color = m_primitive->getMaterial()->shadePath(hit);
						break;
				}

			}else{
				
				hit.color = m_primitive->getColor(hit.hitPoint);	
			}
	}
	
	return hit;
}

Color Scene::traceRay(Ray& ray){
	
	ray.depth++;
	if ((ray.depth) == m_maximumDepth + 1){

		return    m_background;

	}else{

		return hitObjects(ray).color;
	}
}

Color Scene::traceRay(Ray& ray, Color pathWeight){

	ray.depth++;
	if ((ray.depth) == m_maximumDepth + 1){

		return    m_background;

	}else{

		return hitObjects(ray).color;
	}
}