#ifndef _VIEW_PLANE_
#define _VIEW_PLANE_

class ViewPlane {
public:
	int 			hres;   					// horizontal image resolution 
	int 			vres;   					// vertical image resolution
	float			s;							// pixel size



public:

	ViewPlane();   								// default Constructor
	ViewPlane(int hres, int vres, float s);

	~ViewPlane();   							// destructor

	
};


#endif
