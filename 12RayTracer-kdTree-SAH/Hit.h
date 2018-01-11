#ifndef _HIT_
#define _HIT_

#include "Color.h"

class Hit {
public:
	bool		hitObject;		// Did the ray hit an object?
	double			    t;
	Color			color;

	Hit();
	~Hit();
};



#endif