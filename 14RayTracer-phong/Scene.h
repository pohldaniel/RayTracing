#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <iostream>
#include <memory>

#include "ViewPlane.h"
#include "Bitmap.h"
#include "Primitive.h"
#include "Light.h"
#include "Hit.h"

class Scene {

public:

	ViewPlane m_vp;
	std::unique_ptr<Bitmap> m_bitmap;

	Scene();

	Scene(const ViewPlane &vp, const Color &background);

	

	void addPrimitive(Primitive* primitive);
	void addLight(Light* light);

	Hit hitObjects(Ray& ray)const;

	void setViewPlane(const ViewPlane &vp);
	void setPixel(const int x, const int y, Color& color) const;
	
private:
	std::vector<std::shared_ptr<Primitive>>	m_primitives;
	std::vector<std::unique_ptr<Light>>	m_lights;
	Color m_background;
};

#endif // _SCENE_H