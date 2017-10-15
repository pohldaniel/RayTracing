#ifndef _SPHERE_H
#define _SPHERE_H

#include "Vector.h"
#include "Ray.h"
#include "Color.h"

class Sphere{
public:
	Vector3f position;
	Color color;
	double radius;
	double radiusquad;
	
	Sphere(Vector3f position, double radius, Color color);
	~Sphere();


	bool Sphere::RaySphereIntersection(Ray ray, float &t) const;
	Vector3f getNormal(const Vector3f& pi) const;

	const Color &getColor() const;
};

#endif