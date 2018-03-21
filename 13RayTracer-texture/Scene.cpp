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


Scene::Scene(const ViewPlane vp){

	Scene::vp = vp;

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


void Scene::setPixel(const int x, const int y, Color& color)const {
	color.clamp();
	int r = (int)(color.r * 255.0);
	int g = (int)(color.g * 255.0);
	int b = (int)(color.b * 255.0);

	//std::cout << r << "   " << g << "   " << b << std::endl;

	bitmap->setPixel24(x, y, r, g, b);
}




Hit Scene::hitObjects(Ray& _ray)const  {

	Hit		 hit;
	float	 tmin = FLT_MAX;
	Color	 color;
	Ray		 ray;

	hit.color = Color(0.0, 0.0, 0.0);

	for (int j = 0; j < primitives.size(); j++){

		if (primitives[j]->orientable){


			//Construct a Vector(_ray.origin.x, _ray.origin.y, _ray.origin.z, 1.0 )
			ray = Ray((Vector4f(_ray.origin)    * primitives[j]->T),
				(Vector4f(_ray.direction) * primitives[j]->T).normalize());

		}
		else{

			ray = Ray(_ray.origin, _ray.direction.normalize());


		}

		primitives[j]->hit(ray, hit);

		if (hit.hitObject && hit.t < tmin) {

			hit.color = primitives[j]->getColor(ray.origin + ray.direction*hit.t);

			tmin = hit.t;

		}

	}//end for

	return hit;
}