#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "Ray.h"
#include "Hit.h"
#include "Color.h"

class Primitives{
public:
	Primitives();
	Primitives(const Color& color);
	~Primitives();

	virtual bool hit(const Ray& ray, Hit &hit) = 0;

	Color getColor();

private:
	Color color;
};

//////////////////////////////////////////////////////////////////
class Sphere : public Primitives{

public:
	
	Sphere();
	Sphere(Vector3f position, double radius, Color color);
	~Sphere();

	bool hit(const Ray &ray, Hit &hit);

	

private:

	Vector3f position;
	double radius;
};
///////////////////////////////////////////////////////////////////
class Plane : public Primitives{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	bool hit(const Ray &ray, Hit &hit);



private:

	Vector3f normal;
	float distance;
};
#endif