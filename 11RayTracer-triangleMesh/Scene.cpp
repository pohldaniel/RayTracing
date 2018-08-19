#include "Scene.h"

Scene::Scene(){

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


Hit Scene::hitObjects(Ray& _ray)const  {

	Hit		 hit;
	float	 tmin = 1.0E10;
	Color	 color;
	Ray		 ray;


	for (int j = 0; j < primitives.size(); j++){

		if (primitives[j]->orientable){

			ray = Ray(primitives[j]->invT * (Vector4f(_ray.origin, 1.0)),
				(primitives[j]->invT * Vector4f(_ray.direction, 0.0)).normalize());

		}
		else{

			ray = Ray(_ray.origin, _ray.direction.normalize());
		}


		if (primitives[j]->hit(ray, hit) && hit.t < tmin) {

			hit.color = primitives[j]->color;
			tmin = hit.t;
		}
			
	}//end for
	
	return hit;
}


void Scene::setPixel(const int x, const int y, Color& color)const {
	color.clamp();
	int r = (int)(color.r * 255.0);
	int g = (int)( color.g * 255.0);
	int b = (int)(color.b * 255.0);
	

	bitmap->setPixel24(x, y, r, g, b);
}