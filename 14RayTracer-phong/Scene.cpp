#include <iostream>

#include "scene.h"


Scene::Scene() {
	vp = ViewPlane();
	bitmap = NULL;
	try {

		bitmap = new Bitmap(vp.vres, vp.hres, 24);
	}
	catch (const char* e) {
		MessageBox(NULL, e, "Error", MB_OK);

	}

	
}


Scene::Scene(const ViewPlane vp, const Color &background){

	Scene::vp = vp;
	Scene::background = background;
	Scene::bitmap = NULL;

	try {

		bitmap = new Bitmap(vp.vres, vp.hres, 24);
	}
	catch (const char* e) {
		MessageBox(NULL, e, "Error", MB_OK);

	}


}

Scene::~Scene()
{

	if (Scene::bitmap)
	{

		delete Scene::bitmap;
		Scene::bitmap = NULL;
	}



	for (int j = 0; j < primitives.size(); j++){

		delete primitives[j];
		primitives[j] = NULL;
	}




}


void Scene::addPrimitive(Primitive* primitive) {
	primitives.push_back(primitive);
}

void Scene::addLight(Light* light) {
	lights.push_back(light);
}


void Scene::setPixel(const int x, const int y, Color& color)const {
	color.clamp();
	int r = (int)(color.r * 255.0);
	int g = (int)(color.g * 255.0);
	int b = (int)(color.b * 255.0);

	//std::cout << r << "   " << g << "   " << b << std::endl;

	bitmap->setPixel24(x, y, r, g, b);
}




Hit Scene::hitObjects(Ray& _ray)const  {
	
	float	 tmin = FLT_MAX;
	Hit		 hit;
	Ray		 ray;
	
	Color light;
	Vector3f hitPoint;
	Vector3f hitPoint2;

	hit.color = background;

	for (int j = 0; j < primitives.size() ; j++){

		if (primitives[j]->orientable){

			ray = Ray(primitives[j]->invT * (Vector4f(_ray.origin, 1.0) ),
				(primitives[j]->invT * Vector4f(_ray.direction, 0.0)).normalize());
			
		}else{

			ray = Ray(_ray.origin, _ray.direction.normalize());
		}
		
		primitives[j]->hit(ray, hit);
		
		if (hit.hitObject && hit.t < tmin) {

			hitPoint = ray.origin + ray.direction*hit.t;
			
			
			if (primitives[j]->getMaterial()){
			
				Color ambiente(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
				

				for (int i = 0; i < lights.size(); i++){

					if (lights[i]->m_ambient){
						// I_in * k_ambiente
						ambiente = ambiente + *lights[i]->m_ambient;

					}

					if (lights[i]->m_diffuse){
						// I_in * k_diffuse * (L * N)
						diffuse = diffuse + (*lights[i]->m_diffuse * lights[i]->calcDiffuse(hitPoint, primitives[j]->getNormal(hitPoint)));

					}

					if (lights[i]->m_specular && primitives[j]->getMaterial()->m_shinies > 0){
						// I_in * k_specular * (R * V)^20
						specular = specular + (*lights[i]->m_diffuse * lights[i]->calcSpecular(hitPoint, primitives[j]->getNormal(hitPoint), ray.direction,
							primitives[j]->getMaterial()->m_shinies));

					}
				}

					if (primitives[j]->getMaterial()->m_ambient2){
						
						ambiente = ambiente * *primitives[j]->getMaterial()->m_ambient2;
					}

					if (primitives[j]->getMaterial()->m_diffuse2){
						
						diffuse = diffuse * *primitives[j]->getMaterial()->m_diffuse2;
					}

					if (primitives[j]->getMaterial()->m_specular2){
						
						specular = specular * *primitives[j]->getMaterial()->m_specular2;
					}
					
					

					light = primitives[j]->getColor(hitPoint) *( ambiente + diffuse + specular);
				

				hit.color =  light;

			}else {
				
				
				hit.color = primitives[j]->getColor(hitPoint);

			}
				tmin = hit.t;
			
		}
		
	}//end for

	return hit;
}