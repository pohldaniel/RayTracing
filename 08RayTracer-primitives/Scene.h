#ifndef _SCENE_H
#define _SCENE_H

#include <vector>

#include "ViewPlane.h"
#include <iostream>

#include "Bitmap.h"
#include "Camera.h"
#include "Primitives.h"
#include "Hit.h"



class Scene {

public:
	
	ViewPlane vp;
	Bitmap *bitmap;
	std::vector<Primitives*>	primitives;

public:

	Scene();

	Scene(const ViewPlane vp);

	~Scene();

	void addSphere(Sphere* sphere);
	void addPrimitive(Primitives* primitive);

	Hit hitObjects(const Ray& ray)const ;

	void setViewPlane(const ViewPlane &vp);
	void setPixel(const int x, const int y, Color& color) const;
	
};

#endif // _SCENE_H