#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <memory>
#include <string>

#include "Vector.h"
#include "Color.h"
#include "Texture.h"
#include "Hit.h"

class Material{

	friend class Model;
	friend class Mesh;
	friend class ModelIndexed;
	friend class MeshIndexed;
	friend class Scene;
	friend class Reflective;
	

public:
	Material();
	Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);
	Material(const std::shared_ptr<Material> material);
	~Material();

	void setAmbient(const Color &ambient);
	void setAmbient(float ambient);

	void setDiffuse(const Color &diffuse);
	void setDiffuse(float diffuse);

	void setSpecular(const Color &specular);
	void setSpecular(float specular);

	void setColor(const Color &color);
	void setColor(float color);

	void setShinies(const int shinies);

	virtual Color shade(Hit &hit) = 0;
	virtual Color shadeAreaLight(Hit &hit);

	void setNormalTexture(ImageTexture* normalMap);
	void setTexture(Texture* texture);

	Vector3f Bump(Hit &hit);

protected:
	Color m_ambient;
	Color m_diffuse;
	Color m_specular;
	int m_shinies;

	std::shared_ptr<ImageTexture> m_normalMap;
	Matrix4f getTBN(const Hit &hit);	
private:
	std::string colorMapPath;
	std::string bumpMapPath;
	
};
////////////////////////////////////////////////////Phong/////////////////////////////////////////////////////////////////
class Phong : public Material{

public:
	Phong();
	Phong(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);
	Phong(const std::shared_ptr<Material> material);
	~Phong();

	Color shade(Hit &hit);
	
private:

	float calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);
	float calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);
};
////////////////////////////////////////////////////Matte/////////////////////////////////////////////////////////////////
class Matte : public Material{

public:

	Matte();
	Matte(const Color &ambient, const Color &diffuse, const Color &specular);
	Matte(const std::shared_ptr<Material> material);
	~Matte();
	Color shade(Hit &hit);
	Color shadeAreaLight(Hit &hit);

	void setKd(const float kd);
	void setKa(const float ka);

private:

	float m_kd;
	float m_ka;
};
////////////////////////////////////////////////////SVMatte/////////////////////////////////////////////////////////////////
class SVMatte : public Material{

public:

	SVMatte();
	SVMatte(const Color &ambient, const Color &diffuse, const Color &specular);
	SVMatte(const std::shared_ptr<Material> material);
	~SVMatte();
	Color shade(Hit &hit);
	Color shadeAreaLight(Hit &hit);

	void setKd(const float kd);
	void setKa(const float ka);

private:

	float m_kd;
	float m_ka;
};
///////////////////////////////////////////////////Reflective///////////////////////////////////////////////////////////////
class Reflective : public Material{

public:

	Reflective();
	Reflective(const Color &ambient, const Color &diffuse, const Color &specular);
	Reflective(const std::shared_ptr<Material> material);
	~Reflective();
	Color shade(Hit &hit);
	

	void setReflectionColor(const Color &reflectionColor);
	void setReflectionColor(float reflectionColor);
	void setFrensel(float frensel);

private:
	Color m_reflectionColor;
	float m_frensel;
	std::shared_ptr<Phong> m_phong;
};
////////////////////////////////////////////////////Emissive/////////////////////////////////////////////////////////////////
class Emissive : public Material{

public:

	Emissive();
	Emissive(const Color &ambient, const Color &diffuse, const Color &specular);
	Emissive(const std::shared_ptr<Material> material);
	~Emissive();

	Color getLe(Hit &hit) const;
	Color shade(Hit &hit);
	Color shadeAreaLight(Hit &hit);

	void setScaleRadiance(const float radiance);

private:

	float m_ls;

};
#endif