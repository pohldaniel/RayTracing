#ifndef _HIT_H
#define _HIT_H

#include <memory>
#include <cfloat>
#include "Vector.h"
#include "Color.h"
#include "Ray.h"


class Scene;
//class Primitive;

class Hit {

public:
	bool hitObject;		// Did the ray hit an object?
	double t;
	float u;
	float v;
	Color color;
	Vector3f normal;
	Vector3f tangent;
	Vector3f bitangent;
	Vector3f hitPoint;
	Ray originalRay;
	Ray transformedRay;
	//std::shared_ptr<Primitive> primitive;
	std::shared_ptr<Scene> scene;
	
	Hit();
	
	~Hit();
};



#endif