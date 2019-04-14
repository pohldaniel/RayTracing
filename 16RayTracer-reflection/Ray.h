#ifndef _RAY_H
#define _RAY_H

#include "Vector.h"
#include "Color.h"

class Ray{

public:
	Vector3f origin, direction;
	int depth = 0;
	

	Ray();
	Ray(Vector3f& origin, Vector3f& direction);
	Ray(Vector3f& origin, Vector3f& direction, int depth);
	
	~Ray();

};

#endif