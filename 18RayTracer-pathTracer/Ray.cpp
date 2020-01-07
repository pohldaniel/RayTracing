#include "Ray.h"

Ray::~Ray(){}

Ray::Ray(){
	Ray::origin = Vector3f(0.0, 0.0, 0.0);
	Ray::direction = Vector3f(0.0, 0.0, -1.0);
	Ray::depth = 0;	
}

Ray::Ray(Vector3f &origin, Vector3f &direction){
	Ray::origin = origin;
	Ray::direction = direction;
	Ray::depth = 0;
}

Ray::Ray(Vector3f &origin, Vector3f &direction, int a_depth){
	Ray::origin = origin;
	Ray::direction = direction;
	depth = a_depth;
}
////////////////////////////////////////////RayDifferantials//////////////////////////////////////
RayDifferential::~RayDifferential(){}

RayDifferential::RayDifferential() : Ray(){
	m_hasDifferentials = false;
}

RayDifferential::RayDifferential(Vector3f& origin, Vector3f& direction) : Ray(origin, direction){
	m_hasDifferentials = false;
}

void RayDifferential::ScaleDifferentials(float s) {
	m_rxOrigin = origin + (m_rxOrigin - origin) * s;
	m_ryOrigin = origin + (m_ryOrigin - origin) * s;
	m_rxDirection = direction + (m_rxDirection - direction) * s;
	m_ryDirection = direction + (m_ryDirection - direction) * s;
}




