#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include "Ray.h"
#include "Hit.h"
#include "Color.h"

class Primitive{
public:
	Primitive();
	Primitive(const Color& color);
	~Primitive();

	virtual bool hit(const Ray& ray, Hit &hit) = 0;

	Color getColor();

private:
	Color color;
};

//////////////////////////////////////////////////////////////////
class Sphere : public Primitive{

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
class Plane : public Primitive{

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