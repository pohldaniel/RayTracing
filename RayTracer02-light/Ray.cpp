#include "Ray.h"


Ray::~Ray(){}



Ray::Ray(Vector3f origin, Vector3f direction)
{
	Ray::origin = origin;
	Ray::direction = direction;

}

Vector3f Ray::getDirection(){

	return direction;
}

Vector3f Ray::getOrigin(){

	return origin;
}