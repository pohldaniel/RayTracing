#include "Ray.h"

Ray::~Ray(){

}

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









