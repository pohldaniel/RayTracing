#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <iostream>
#include <memory>

#include "ViewPlane.h"
#include "Bitmap.h"
#include "Primitive.h"
#include "Light.h"



class Scene {

	friend class Phong;
	friend class NormalMap;
	friend class Projection;
public:

	Scene();
	Scene(const ViewPlane &vp, const Color &background);

	void addPrimitive(Primitive* primitive);
	void addLight(Light* light);
	Hit hitObjects(Ray& ray);

	void setViewPlane(const ViewPlane &vp);
	void setPixel(const int x, const int y, Color& color) const;

	std::shared_ptr<Bitmap> getBitmap();
	ViewPlane getViewPlane();

private:

	std::vector<std::shared_ptr<Primitive>>	m_primitives;
	std::vector<std::unique_ptr<Light>>	m_lights;
	std::shared_ptr<Bitmap> m_bitmap;

	ViewPlane m_vp;
	Color m_background;
	Hit		 m_hit;
};

#endif // _SCENE_H