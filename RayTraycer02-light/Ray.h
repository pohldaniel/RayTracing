#ifndef _RAY_H
#define _RAY_H

#include "Vector.h"

class Ray
{

public:
	Vector3f origin, direction;


	Ray();
	Ray(Vector3f origin, Vector3f direction);
	~Ray();

	Vector3f getDirection();
	Vector3f getOrigin();

};

#endif