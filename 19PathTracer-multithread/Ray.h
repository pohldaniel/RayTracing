#ifndef _RAY_H
#define _RAY_H

#include "Vector.h"
#include "Color.h"

class Ray{

public:
	Ray();
	Ray(const Vector3f& origin, const Vector3f& direction);
	Ray(Vector3f& origin, Vector3f& direction, int depth);
	~Ray();

	Vector3f origin, direction;
	int depth = 0;

};
/////////////////////////////////////////////////////////////////////////////
class RayDifferential : public Ray{

public:
	RayDifferential();
	RayDifferential(Vector3f& origin, Vector3f& direction);
	~RayDifferential();

	void ScaleDifferentials(float s);

	bool m_hasDifferentials;
	Vector3f m_rxOrigin, m_ryOrigin;
	Vector3f m_rxDirection, m_ryDirection;
};
#endif