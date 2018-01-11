#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

//for std::min
#include <algorithm>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

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
	Matrix4f T;
};
////////////////////////////////////////////////////////////////
class OrientablePrimitives : public Primitive {


public:

	OrientablePrimitives();
	OrientablePrimitives(const Color& color);
	~OrientablePrimitives();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);


};

//////////////////////////////////////////////////////////////////
class Sphere : public Primitive{

public:

	Sphere();
	Sphere(Vector3f position, float radius, Color color);
	~Sphere();

	bool hit(const Ray &ray, Hit &hit);



private:

	Vector3f position;
	float radius;
};
///////////////////////////////////////////////////////////////////
class Triangle : public Primitive{

public:
	Triangle();
	Triangle(const Vector3f &normal, const Vector3f &a, const Vector3f &b, const Vector3f &c, Color color);
	~Triangle();
	bool hit(const Ray &ray, Hit &hit);

public:

	Vector3f normal;
	Vector3f a;
	Vector3f b;
	Vector3f c;

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



////////////////////////////////////////////////////////////////////////

class Mesh{
public:
	Mesh();
	~Mesh();

	bool loadObject(const char* filename);



	std::vector<Triangle*> triangles;
};


#endif