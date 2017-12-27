#ifndef _SPHERE_H
#define _SPHERE_H

#include "Vector.h"
#include "Ray.h"

#include "Hit.h"

class Sphere{
public:
	Vector3f position;
	double radius;
	
	Color color;

	bool Sphere::hit(const Ray &ray, const Hit &hit) const;




	Sphere(Vector3f position, double radius, Color color);
	~Sphere();
};

#endif