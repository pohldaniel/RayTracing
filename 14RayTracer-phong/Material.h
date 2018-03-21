#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "Color.h"

class Material
{
public:
	Material();
	Material(float a_ambi, float a_diff, float a_spec, int n);
	~Material();

	void setDiffuse(float a_diff);
	void setSpecular(float a_refl);
	void setAmbiente(float a_ambi);
	void setSurfaceProperty(float a_n);

	float getSpecular();
	float getDiffuse();
	float getAmbiente();
	int getSurfaceProperty();

private:

	float m_spec;
	float m_diff;
	float m_ambi;
	int m_n;			// neede at the function getSpecular() to determine the surface property
};

#endif