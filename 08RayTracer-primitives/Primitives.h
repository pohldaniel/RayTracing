#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "Ray.h"
#include "Hit.h"
#include "Color.h"

class Primitives{
public:
	Primitives();

	~Primitives();

	virtual bool hit(const Ray& ray, Hit &hit) const = 0;

};


class Sphere : public Primitives{

public:
	
	Sphere();
	Sphere(Vector3f position, double radius, Color color);
	~Sphere();

	bool hit(const Ray &ray, const Hit &hit) const;

	

private:

	Vector3f position;
	double radius;
	Color color;

};


#endif