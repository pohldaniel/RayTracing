#include "Primitive.h"

Primitive::Primitive(){

	color = Color(1.0, 0.0, 0.0);
};

Primitive::Primitive(const Color& color){

	Primitive::color = color;
}

Primitive::~Primitive(){

};

Color Primitive::getColor(){

	return color;
}

//////////////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere() :Primitive(){

	Sphere::position = Vector3f(0.0, 0.0, 0.0);
	Sphere::radius = 1;

}

Sphere::Sphere(Vector3f position, double radius, Color color) :Primitive(color){
	Sphere::position = position;
	Sphere::radius = radius;

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

//////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::normal = Vector3f(0.0, 1.0, 0.0);
	Plane::distance = -1.0;

}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitive(color){
	Plane::normal = normal;
	Plane::distance = distance;

}

Plane::~Plane(){}

bool Plane::hit(const Ray &ray, Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(normal, ray.origin)) / Vector3f::dot(normal, ray.direction);

	if (result > 0.0){
		hit.t = result;
		return true;
	}

	return false;
}