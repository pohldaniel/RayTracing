#include <algorithm>

#include "Sphere.h"

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1);
Sphere::~Sphere(){}




Sphere::Sphere(Vector3f position, double radius, Color color){
	Sphere::position = position;
	Sphere::color = color;
	Sphere::radius = radius;
	Sphere::radiusquad = radius * radius;
}




bool Sphere::RaySphereIntersection(Ray ray, float &root) const
{
	float t0, t1; // solutions for t if the ray intersects


	// analytic solution
	Vector3f L = ray.origin - position;
	float a = Vector3f::dot(ray.direction, ray.direction);
	float b = 2 * Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - radius*radius;
	if (!solveQuadratic(a, b, c, t0, t1)) return false;

	//checking the samaller root and clipping field
	if (t0 <= t1){
		if (t0 > 1){
			root = t0;
			return true;
		}
	}

	if (t1 > 1){
		root = t1;
		return true;
	}

	return false;
}


bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) {
		x0 = x1 = -0.5 * b / a;
	}
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}

	return true;
}


Vector3f Sphere::getNormal(const Vector3f& pi) const{
	return (pi - position) / radius;

}

const Color &Sphere::getColor() const{
	return color;
}