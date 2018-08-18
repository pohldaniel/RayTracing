#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

//for std::min
#include <algorithm>

#include "Ray.h"
#include "Hit.h"
#include "Color.h"



class Primitive{

	friend class Scene;

public:
	Primitive();
	Primitive(const Color& color);
	~Primitive();

	virtual bool hit(const Ray& ray, Hit &hit) = 0;

	Color getColor();





protected:
	bool  orientable;
	Color color;
	Matrix4f invT;
};
////////////////////////////////////////////////////////////////




class OrientablePrimitive : public Primitive {


public:

	OrientablePrimitive();
	OrientablePrimitive(const Color& color);
	~OrientablePrimitive();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float);

private:

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




/////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitive{

public:

	Torus();
	Torus(float a, float b, Color color);
	~Torus();

	bool hit(const Ray &ray, Hit &hit);

private:
	float a;
	float b;
};




#endif