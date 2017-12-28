#include <algorithm>

#include "Sphere.h"



Sphere::Sphere(Vector3f position, double radius, Color color) {
	Sphere::position = position;
	Sphere::radius = radius;
	Sphere::color = color;
}

Sphere::~Sphere(){}



bool Sphere::hit(const Ray &ray, Hit &hit) {

	//Use position - origin to get a negative b
	Vector3f L = position - ray.origin;


	float b = Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - radius*radius;

	float d = b*b - c;
	if (d < 0.00001) return false;

	//----------------------------------
	float result = -1.0;

	result = b - sqrt(d);
	if (result < 0.0){

		result = b + sqrt(d);

	}


	if (result > 0.0){

		hit.t = result;
		return true;

	}

	return false;
}