#include <iostream>

#include "scene.h"


Scene::Scene() {
	m_vp = ViewPlane();
	m_bitmap = NULL;

	try {

		m_bitmap = std::unique_ptr<Bitmap>( new Bitmap(m_vp.vres, m_vp.hres, 24));
	}
	catch (const char* e) {
		MessageBox(NULL, e, "Error", MB_OK);

	}

	
}


Scene::Scene(const ViewPlane &vp, const Color &background){

	m_vp = vp;
	m_background = background;
	m_bitmap = NULL;

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




Hit Scene::hitObjects(Ray& _ray)const  {
	
	float	 tmin = FLT_MAX;
	Hit		 hit;
	Ray		 ray;
	
	Color light;
	Vector3f hitPoint;


	hit.color = m_background;

	for (unsigned int j = 0; j < m_primitives.size(); j++){

		if (m_primitives[j]->orientable){

			ray = Ray(m_primitives[j]->invT * (Vector4f(_ray.origin, 1.0)),
				(m_primitives[j]->invT * Vector4f(_ray.direction, 0.0)).normalize());
			
		}else{

			ray = Ray(_ray.origin, _ray.direction.normalize());
		}
		
		m_primitives[j]->hit(ray, hit);
		
		if (hit.hitObject && hit.t < tmin) {
			tmin = hit.t;
			hitPoint = ray.origin + ray.direction*hit.t;
			Vector3f normal = m_primitives[j]->getNormal(hitPoint);
			
			if (m_primitives[j]->getMaterial()){
			
				Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
				

				for (unsigned int i = 0; i < m_lights.size(); i++){

					
						// I_in * k_ambiente
						ambiente = ambiente + m_lights[i]->m_ambient;

						// I_in * k_diffuse * (L * N)
						diffuse = diffuse + (m_lights[i]->m_diffuse * m_lights[i]->calcDiffuse(hitPoint, normal));
						
						// I_in * k_specular * (R * V)^20
						specular = specular + (m_lights[i]->m_specular * m_lights[i]->calcSpecular(hitPoint, normal, ray.direction,
							m_primitives[j]->getMaterial()->m_shinies));

					
				}

				ambiente = ambiente * m_primitives[j]->getMaterial()->m_ambient;
				diffuse = diffuse * m_primitives[j]->getMaterial()->m_diffuse;
				specular = specular * m_primitives[j]->getMaterial()->m_specular;
					
				hit.color = m_primitives[j]->getColor(hitPoint) * (ambiente + diffuse + specular);
				//hit.color = m_primitives[j]->getColor(hitPoint) * (ambiente + diffuse) + specular;
 			}

		}
		
	}//end for

	return hit;
}