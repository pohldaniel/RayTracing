#include <iostream>

#include "scene.h"

Scene::Scene() {

	m_vp = ViewPlane();
	m_background = Color(0.0, 0.0, 0.0);
	m_bitmap = NULL;
	m_hit = Hit(this);
	
	try {

		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(m_vp.vres, m_vp.hres, 24));
	}
	catch (const char* e) {

		std::cout << "Could not load Scene bitmap!" << std::endl;
	}

	
}

Scene::Scene(const ViewPlane &vp, const Color &background){

	m_vp = vp;
	m_background = background;
	m_bitmap = NULL;
	m_hit = Hit(this);

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


void Scene::setPixel(const int x, const int y, Color& color)const {
	color.clamp();
	int r = (int)(color.r * 255.0);
	int g = (int)(color.g * 255.0);
	int b = (int)(color.b * 255.0);

	//std::cout << r << "   " << g << "   " << b << std::endl;

	m_bitmap->setPixel24(x, y, r, g, b);
}

std::unique_ptr<Bitmap> Scene::getBitmap(){

	return std::move(m_bitmap);
}

ViewPlane Scene::getViewPlane(){

	return m_vp;
}

Hit Scene::hitObjects(Ray& _ray)  {
	
	Ray		 ray;
	float	 tmin = FLT_MAX;

	m_hit.t = FLT_MAX;
	m_hit.hitObject = false;
	m_hit.color = m_background;
	
	
	for (unsigned int j = 0; j < m_primitives.size(); j++){

		if (m_primitives[j]->orientable){

			ray = Ray(m_primitives[j]->invT * (Vector4f(_ray.origin, 1.0)),
				(m_primitives[j]->invT * Vector4f(_ray.direction, 0.0)).normalize());
			
		}else{

			ray = Ray(_ray.origin, _ray.direction.normalize());
		}
		
		m_primitives[j]->hit(ray, m_hit);
		

		if (m_hit.hitObject && m_hit.t < tmin) {

			tmin = m_hit.t;
			m_hit.hitPoint = ray.origin + ray.direction* m_hit.t;
			m_hit.normal = m_primitives[j]->getNormal(m_hit.hitPoint);

			if (m_primitives[j]->getMaterial()){
				
				m_hit.color = m_primitives[j]->getColor(m_hit.hitPoint) * m_primitives[j]->getMaterial()->shade(m_hit, ray.direction);
				
			}else{

				m_hit.color = m_primitives[j]->getColor(m_hit.hitPoint);

			}

		}
		
	}//end for

	return m_hit;
}