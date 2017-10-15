#ifndef _SPHERE_H
#define _SPHERE_H

#include "Vector.h"
#include "Ray.h"

class Sphere{
public:
	Vector3f position;
	double radius;
	double radiusquad;

	bool Sphere::RaySphereIntersection(Ray ray) const;




	Sphere(Vector3f position, double radius);
	~Sphere();
};

#endif