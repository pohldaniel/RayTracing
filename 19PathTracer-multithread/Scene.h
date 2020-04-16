#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <iostream>
#include <memory>
#include <cmath>
#include <random>
#include <ctime>


#include "ViewPlane.h"
#include "Bitmap.h"
#include "Primitive.h"
#include "Light.h"



class Scene {

	friend class Phong;
	friend class NormalMap;
	friend class Matte;
	friend class Reflective;
	friend class Emissive;

public:

	enum Tracer { Whitted, AreaLighting, PathTracer, PathTracerIt};

	Scene();
	Scene(const ViewPlane &vp, const Color &background);

	void addPrimitive(Primitive* primitive);
	
	void addLight(Light* light);
	Hit hitObjects(Ray& ray);
	Hit hitObjects2(Ray& ray);

	Hit pathTracerIt(Ray& primaryRay);

	Color traceRay(Ray& ray);
	Color traceRay(Ray& ray, Color pathWeight);
	

	void setPixel(const int x, const int y, Color& color) const;
	void setDepth(int depth);
	void setAmbientLight(AmbientLight *ambient);
	void setTracer(Tracer tracer);

	std::shared_ptr<Bitmap> getBitmap();
	ViewPlane getViewPlane();

	void setSampler(Sampler* sampler);



	std::shared_ptr<Primitive> m_primitive;
	std::vector<std::shared_ptr<Primitive>>	m_primitives;
	std::vector<std::unique_ptr<Light>>	m_lights;
	std::unique_ptr<AmbientLight> m_ambient;
	std::shared_ptr<Bitmap> m_bitmap;
	

	ViewPlane m_vp;
	Color m_background;
	std::shared_ptr<Scene> m_scene;
	int m_maximumDepth;
	Tracer m_tracer;


	std::shared_ptr<Sampler> m_sampler;
	std::default_random_engine m_generator;
	std::uniform_real_distribution<float> m_distribution;
	Vector3f Scene::sampleDirection(Vector3f& normal);
	Vector3f Scene::sampleDirection2(Vector3f& normal);
	
};

#endif // _SCENE_H