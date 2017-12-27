#ifndef _LIGHT_H
#define _LIGHT_H


#include "Vector.h"
#include "Color.h"

class Light
{
public:
	Light(Vector3f pos, Color col);
	~Light();

	Vector3f position;
	Color color;

	const Color &getColor() const;
};

#endif