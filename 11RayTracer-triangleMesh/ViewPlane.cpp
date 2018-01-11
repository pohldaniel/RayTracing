#include "ViewPlane.h"


ViewPlane::ViewPlane(){

	hres				= 640;
	vres				= 480;
	s					= 1.0;
	
}

ViewPlane::ViewPlane(int hres,
					 int vres,
					 float s){

	ViewPlane::hres				 = hres;
	ViewPlane::vres				 = vres;
	ViewPlane::s				 = s;

}


ViewPlane::~ViewPlane()
{
}