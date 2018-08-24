#ifndef _HIT_H
#define _HIT_H

#include <cfloat>
#include "Vector.h"
#include "Color.h"

class Hit {

public:
	bool hitObject;		// Did the ray hit an object?
	double t;
	Color color;
	

	Hit();
	~Hit();
};



#endif