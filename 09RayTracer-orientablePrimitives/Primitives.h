#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

//for std::min
#include <algorithm>

#include "Ray.h"
#include "Hit.h"
#include "Color.h"

class Scene;

class Primitives{

	friend class Scene;

public:
	Primitives();
	Primitives(const Color& color);
	~Primitives();

	virtual bool hit(const Ray& ray, Hit &hit) = 0;

	Color getColor();


	


protected:
	bool  orientable;
	Color color;
	Matrix4f T;
};
////////////////////////////////////////////////////////////////




class OrientablePrimitives : public Primitives {


public:

	OrientablePrimitives();
	OrientablePrimitives(const Color& color);
	~OrientablePrimitives();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	
	
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




/////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitives{

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