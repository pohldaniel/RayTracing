#include "Primitives.h"

Primitives::Primitives(){
	orientable = false;
	color = Color(1.0, 0.0, 0.0);
	Primitives::T.identity();
}

Primitives::Primitives(const Color& color){
	orientable = false;
	Primitives::color = color;
	Primitives::T.identity();
}

Primitives::~Primitives(){

}

Color Primitives::getColor(){

	return color;
}

//////////////////////////////////////////////////////////////////////////////////////////


OrientablePrimitives::OrientablePrimitives():Primitives(){
	orientable = true;
	
}


OrientablePrimitives::OrientablePrimitives(const Color& color ) : Primitives(color){
	orientable = true;
	
}

OrientablePrimitives::~OrientablePrimitives(){
	
	
}

void OrientablePrimitives::rotate(const Vector3f &axis, float degrees){

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degrees);

	T = rotMtx *T;


}

void OrientablePrimitives::translate(float dx, float dy, float dz){

	Matrix4f transMtx;
	transMtx.translate(dx, dy, dz);
	T = transMtx *T;
	//T *= transMtx;
}

//////////////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere():Primitives(){

	Sphere::position = Vector3f(0.0, 0.0, 0.0);
	Sphere::radius = 1;
	
}

Sphere::Sphere(Vector3f position, double radius, Color color):Primitives(color){
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
Plane::Plane() :Primitives(){

	Plane::normal = Vector3f(0.0, 1.0, 0.0);
	Plane::distance = -1.0;

}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitives(color){
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

//////////////////////////////////////////////////////////////////////////////////////////////////////

Torus::Torus():OrientablePrimitives(){

	Torus::a = 1.0;
	Torus::b = 0.5;
}

Torus::Torus(float a, float b, Color color) :OrientablePrimitives( color){

	Torus::a = a;
	Torus::b = b;
}

Torus::~Torus(){

}

bool Torus::hit(const Ray &ray, Hit &hit){

	float Ra2 = Torus::a*Torus::a;
	float ra2 = Torus::b*Torus::b;

	
	float m = Vector3f::dot(ray.origin, ray.origin);
	float n = Vector3f::dot(ray.origin, ray.direction);

	float k = (m - ra2 - Ra2) / 2.0;
	float a = n;												
	float b = n*n + Ra2*ray.direction[0]*ray.direction[0] + k;
	float c = k*n + Ra2*ray.origin[0]*ray.direction[0];			
	float d = k*k + Ra2*ray.origin[0]*ray.origin[0] - Ra2*ra2;	

	//----------------------------------

	float p = -3.0*a*a + 2.0*b;
	float q = 2.0*a*a*a - 2.0*a*b + 2.0*c;
	float r = -3.0*a*a*a*a + 4.0*a*a*b - 8.0*a*c + 4.0*d;
	p /= 3.0;
	r /= 3.0;
	float Q = p*p + r;
	float R = 3.0*r*p - p*p*p - q*q;

	float h = R*R - Q*Q*Q;
	float z = 0.0;

	// discriminant > 0
	if (h < 0.0)
	{
		float sQ = sqrt(Q);
		z = 2.0*sQ*cos(acos(R / (sQ*Q)) / 3.0);
	}
	// discriminant < 0
	else
	{
		float sQ = pow(sqrt(h) + abs(R), 1.0 / 3.0);
		if (R < 0.0){
			z = -(sQ + Q / sQ);
		}
		else{
			z = (sQ + Q / sQ);
		}
	}

	z = p - z;

	//----------------------------------

	float d1 = z - 3.0*p;
	float d2 = z*z - 3.0*r;
	float d1o2 = d1 / 2.0;

	if (abs(d1)< 0.0001)
	{
		if (d2<0.0) return false;
		d2 = sqrt(d2);
	}
	else
	{
		if (d1<0.0) return false;
		d1 = sqrt(d1 / 2.0);
		d2 = q / d1;
	}

	//----------------------------------

	float result = -1.0;
	float result2 = -1.0;

	float t1;
	float t2;

	h = d1o2 - z + d2;
	if (h>0.0)
	{
		h = sqrt(h);

		t1 = -d1 - h - a;
		t2 = -d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result = std::min(t1, t2);
		}
		else if (t1 > 0) {
			result = t1;
		}
		else if (t2 > 0){
			result = t2;
		}

	}


	h = d1o2 - z - d2;
	if (h>0.0)
	{
		h = sqrt(h);
		t1 = d1 - h - a;
		t2 = d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result2 = std::min(t1, t2);
		}
		else if (t1 > 0) {
			result2 = t1;
		}
		else if (t2 > 0){
			result2 = t2;
		}
	}

	if (result2 > 0.0 && result > 0.0){
		result = std::min(result, result2);

	}
	else if (result2 > 0.0) result = result2;



	if (result > 0.0){
		hit.t = result;
		return true;
	}

return false;
}