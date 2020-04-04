#ifndef _VIEW_PLANE_
#define _VIEW_PLANE_

class ViewPlane {

	friend class Scene;
	friend class Orthographic;
	friend class Projection;
	friend class Pinhole;
	friend class FishEye;
	friend class Spherical;
	friend class ThinLens;

public:

	ViewPlane();   								
	ViewPlane(int hres, int vres, float s);
	~ViewPlane();   							

private:
	int 			hres;   					// horizontal image resolution 
	int 			vres;   					// vertical image resolution
	float			s;							// pixel size
};


#endif
