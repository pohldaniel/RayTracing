#ifndef _LIGHT_H
#define _LIGHT_H


#include "Vector.h"
#include "Color.h"
#include "Material.h"

class Light{
	
	friend class Phong;
	friend class NormalMap;
	friend class Matte;
	friend class Reflective;
	
public:
	
	Light();
	Light(const Vector3f &pos, const Color &ambiente, const Color &diffuse, const Color &specular);
	virtual ~Light();

	virtual Vector3f getDirection(const Vector3f &hitPoint);
	virtual Color L(const Vector3f &pos);

protected:

	Vector3f m_position;
	Color m_ambient;
	Color m_diffuse;
	Color m_specular;	
};
/////////////////////////////////////////////////////////////////////////////
class AmbientLight : public Light {

public:
	
	AmbientLight();
	AmbientLight(const Color &color);
	~AmbientLight();

	Color L(const Vector3f &pos);

	void setScaleRadiance(const float radiance);

private:

	float 	 m_ls;
};
/////////////////////////////////////////////////////////////////////////////
class DirectionalLight : public Light {

public:

	DirectionalLight();
	DirectionalLight(const Vector3f &direction, const Color &ambiente, const Color &diffuse, const Color &specular);
	DirectionalLight(const Vector3f &direction, const Color &color);
	~DirectionalLight();


	Vector3f getDirection(const Vector3f &hitPoint);
	Color L(const Vector3f &pos);

	void setDirection(const Vector3f &direction);
	void setScaleRadiance(const float radiance);


private:

	float 	 m_ls;
	Color 	 m_color;
	Vector3f m_direction;

};
/////////////////////////////////////////////////////////////////////////////
class PointLight : public Light {

public:

	PointLight();
	PointLight(const Vector3f &pos, const Color &ambiente, const Color &diffuse, const Color &specular);
	PointLight(const Vector3f &pos, const Color &color);
	~PointLight();


	Vector3f getDirection(const Vector3f &hitPoint);
	Color L(const Vector3f &pos);

	void setPosition(const Vector3f &pos);
	void setScaleRadiance(const float radiance);


private:

	float 	 m_ls;
	Color 	 m_color;
	
};
#endif