#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <iostream>


#include "ViewPlane.h"
#include "Bitmap.h"
#include "Primitive.h"
#include "Light.h"
#include "Hit.h"

class Scene {

public:

	ViewPlane vp;
	Bitmap *bitmap;
	
public:

	Scene();

	Scene(const ViewPlane vp, const Color &background);

	~Scene();

	void addPrimitive(Primitive* primitive);
	void addLight(Light* light);

	Hit hitObjects(Ray& ray)const;

	void setViewPlane(const ViewPlane &vp);
	void setPixel(const int x, const int y, Color& color) const;

private:
	std::vector<Primitive*>	primitives;
	std::vector<Light*>	lights;
	Color background;
};

#endif // _SCENE_H