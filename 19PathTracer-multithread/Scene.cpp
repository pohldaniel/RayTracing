#include <iostream>

#include "scene.h"

Scene::Scene() : m_generator(std::random_device()()), m_distribution(0.0, 1.0){

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

Scene::Scene(const ViewPlane &vp, const Color &background) : m_generator(std::random_device()()), m_distribution(0.0, 1.0){

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
	
	if (m_tracer == Tracer::PathTracerIt){
		
		return pathTracerIt(_ray);
	}

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
					case PathTracerIt:
						hit.color = pathTracerIt(_ray).color;
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

void Scene::setSampler(Sampler* sampler){

	m_sampler = std::shared_ptr<Sampler>(sampler);
}

Vector3f Scene::sampleDirection(Vector3f& normal) {

	Vector3f w = normal;
	Vector3f v = Vector3f::cross(Vector3f(0.0034f, 1.0, 0.0071), w);
	Vector3f::normalize(v);
	Vector3f u = Vector3f::cross(v, w);

	Vector3f sp = m_sampler->sampleHemisphere();

	return u*sp[0] + v*sp[1] + w*sp[2];
}

Vector3f Scene::sampleDirection2(Vector3f& normal){

	Vector3f nt = std::fabs(normal[0]) > std::fabs(normal[1]) ? Vector3f(normal[2], 0, -normal[0]).normalize() : Vector3f(0, -normal[2], normal[1]).normalize();
	Vector3f nb = Vector3f::cross(normal, nt);

	float r1 = m_distribution(m_generator);
	float phi = 2 * PI * m_distribution(m_generator);
	float sinTheta = sqrtf(1 - r1 * r1);
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);

	return Vector3f(nb * x + normal * r1 + nt * z);
}

float PdfW(Vector3f& normal, Vector3f& sampledDirection){

	return max(1e-6f, max(0, Vector3f::dot(normal, sampledDirection)) / PI);
}



Hit Scene::pathTracerIt(Ray& primaryRay){

	Ray ray = primaryRay;
	
	float	 tmin = FLT_MAX;
	Hit		 hit;

	hit.color = m_background;
	hit.scene = m_scene;
	hit.originalRay = ray;
	m_primitive = NULL;
	bool hitObject = false;

	float cosAtCamera = Vector3f::dot(ray.direction, Vector3f(0.0, 0.0, 1.0).normalize());
	Color pathWeight = Color(1.0, 1.0, 1.0) ;
	Color hitColor;
	

	int maxPathLength = m_maximumDepth;
	for (int i = 0; i < maxPathLength; i++){

		for (unsigned int j = 0; j < m_primitives.size(); j++){
			hit.transformedRay = ray;
			m_primitives[j]->hit(hit);

			if (hit.hitObject && hit.t < tmin) {
				tmin = hit.t;
				m_primitive = m_primitives[j];
				hitObject = true;
			}
		}

		if (!hitObject){
			
			hit.color = m_background;
			break;

		}else{

			hit.t = FLT_MAX;
			hit.hitPoint = ray.origin + ray.direction * tmin;
			hit.normal = m_primitive->getNormal(hit.hitPoint);
			hitColor = m_primitive->getColor(hit.hitPoint);
			
			AreaLight* light = static_cast<AreaLight*>(m_lights[0].get());
			
			if (light->m_primitive == m_primitive ){
				
				hit.color = pathWeight * hitColor * (1.0 / light->pdf(hit));
				break;
			}

			/*float continuationPdf = min(1, hitColor.Max());
			if (m_distribution(m_generator) >= continuationPdf){

				hit.color = m_background; // Absorbation
				break;
			}*/
			
			Vector3f newDirection = sampleDirection(hit.normal);
			float pdf = max(1e-6f, max(0, Vector3f::dot(hit.normal, newDirection)) * invPI);
			float lambert = Vector3f::dot(hit.normal, newDirection);
			pathWeight = pathWeight * hitColor *  (lambert / pdf) * invPI  * 0.6;
			ray = Ray(hit.hitPoint, newDirection);

		}

	}
	
	
	return hit;
}