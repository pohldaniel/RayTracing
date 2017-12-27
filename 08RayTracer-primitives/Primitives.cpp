#include "Primitives.h"

Primitives::Primitives(){

};

Primitives::~Primitives(){

};

/////////////////////////////////////

Sphere::Sphere(){

	Sphere::position = Vector3f(0.0, 0.0, 0.0);
	Sphere::radius = 1;
	Sphere::color = Color(1.0, 0.0, 0.0);
}

Sphere::Sphere(Vector3f position, double radius, Color color){
	Sphere::position = position;
	Sphere::radius = radius;
	Sphere::color = color;
}


bool Sphere::hit(const Ray &ray, const Hit &hit) const{

	Vector3f L = ray.origin - position;
	float a = Vector3f::dot(ray.direction, ray.direction);
	float b = 2 * Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - radius*radius;

}