#ifndef _SPHERE_H
#define _SPHERE_H

#include "Vector.h"
#include "Ray.h"
#include "Hit.h"

class Sphere {
	friend class Scene;
public:

	Sphere(Vector3f position, double radius, Color color);
	~Sphere();

	bool hit(const Ray &ray, Hit &hit);



private:
	Color color;
	Vector3f position;
	double radius;
};

#endif