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

	
public:
	Material();
	Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);
	Material(const std::shared_ptr<Material> material);
	~Material();

	void setAmbient(const Color &ambient);
	void setDiffuse(const Color &diffuse);
	void setSpecular(const Color &specular);
	void setshinies(const int shinies);

	virtual Color shade(Hit &hit, const Vector3f &w_0) = 0;

protected:
	Color m_ambient;
	Color m_diffuse;
	Color m_specular;
	int m_shinies;

private:
	std::string colorMapPath;
	std::string bumpMapPath;
	
};

class Phong : public Material{

public:
	Phong();
	Phong(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);
	Phong(const std::shared_ptr<Material> material);
	~Phong();

	Color shade(Hit &hit, const Vector3f &w_0);
	
private:

	float calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);
	float calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);
};


class NormalMap : public Material{

	

public:

	NormalMap();
	NormalMap(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);
	NormalMap(const std::shared_ptr<Material> material);
	NormalMap(const char* path);
	~NormalMap();

	Color shade(Hit &hit, const Vector3f &w_0);
	void setNormalMap(std::shared_ptr<Texture> normalMap);
private:
	
	std::shared_ptr<Texture> m_normalMap;
	float calcDiffuse(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);
	float calcSpecular(const Hit &hit, const Vector3f &w_0, const Vector3f &w_i);

	Matrix4f getTBN(const Hit &hit);
};

#endif