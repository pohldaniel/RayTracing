#ifndef _LIGHT_H
#define _LIGHT_H


#include "Vector.h"
#include "Color.h"
#include "Material.h"

class Light{
	
	friend class Phong;
	friend class NormalMap;

public:
	
	Light();
	Light(const Vector3f &pos, const Color &ambiente, const Color &diffuse, const Color &specular);
	~Light();

private:

	Vector3f m_position;
	Color m_ambient;
	Color m_diffuse;
	Color m_specular;

	
};

#endif