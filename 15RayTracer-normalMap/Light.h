#ifndef _LIGHT_H
#define _LIGHT_H


#include "Vector.h"
#include "Color.h"

class Light
{
public:

	Light();
	Light(const Vector3f &pos, const Color &ambiente, const Color &diffuse, const Color &specular);
	~Light();

	Vector3f m_position;


	Color m_ambient;
	Color m_diffuse;
	Color m_specular;

	float calcDiffuse(const Vector3f& a_Pos, const Vector3f& a_Normal);
	float calcSpecular(const Vector3f& a_Pos, const Vector3f& a_Normal, const Vector3f& a_viewDirection, const int a_n);
};

#endif