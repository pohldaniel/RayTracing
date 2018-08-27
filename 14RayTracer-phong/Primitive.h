#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>

#include "Material.h"
#include "Texture.h"
#include "Vector.h"
#include "Color.h"
#include "Hit.h"
#include "Ray.h"


class BBox{
public:
	BBox() : m_pos(Vector3f(0, 0, 0)), m_size(Vector3f(0, 0, 0)) {};
	BBox(Vector3f& a_pos, Vector3f& a_size) : m_pos(a_pos), m_size(a_size)  {};
	Vector3f& getPos() { return m_pos; }
	Vector3f& getSize() { return m_size; }

	inline void extend(Vector3f a){

		m_pos = Vector3f::Min(a, m_pos);
		m_size = Vector3f::Max(a, m_size);
	}

	inline float getSurfaceArea(){

		float xdim = m_size[0] - m_pos[0];
		float ydim = m_size[1] - m_pos[1];
		float zdim = m_size[2] - m_pos[2];

		return 2 * (xdim*ydim + ydim*zdim + zdim*xdim);
	}


	bool intersect(const Ray &ray);

	Vector3f m_pos, m_size;
	float m_tmin, m_tmax;
};

/////////////////////////////////////////////////////////////////////////////

class Primitive {

	friend class Scene;
	friend class KDTree;
	friend class Model;

public:
	Primitive();

	Primitive(const Color &color);
	virtual ~Primitive();

	virtual void hit(const Ray& ray, Hit &hit) = 0;
	virtual void calcBounds() = 0;
	virtual Color getColor(const Vector3f& a_Pos) = 0;
	virtual Vector3f getNormal(const Vector3f& a_Pos) = 0;
	virtual void clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox);

	BBox& getBounds();

	void setTexture(Texture* texture);
	std::shared_ptr<Texture> getTexture();
	
	void setMaterial(Material* material);
	virtual std::shared_ptr<Material> getMaterial();

	

protected:

	bool orientable;
	bool bounds;	
	
	
	Matrix4f T;
	Matrix4f invT;
	BBox box;


	std::shared_ptr<Material> m_material;
	std::shared_ptr<Texture> m_texture;
	Color m_color;
	
};
/////////////////////////////////////////////////////////////////////////////
class OrientablePrimitive : public Primitive {


public:

	OrientablePrimitive();
	OrientablePrimitive(const Color& color);
	~OrientablePrimitive();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float);

};

/////////////////////////////////////////////////////////////////////////////

class Triangle :public OrientablePrimitive{

public:
	
	Triangle(const Vector3f &a_V1, const Vector3f &a_V2, const Vector3f &a_V3, const Color &color, const bool cull);
	~Triangle();

	
	void hit(const Ray& ray, Hit &hit);
	void calcBounds();
	Color getColor(const Vector3f& a_Pos);
	Vector3f getNormal(const Vector3f& a_Pos);

	void setUV(float a_u1, float  a_u2, float  a_u3, float a_v1, float  a_v2, float  a_v3){

		u1 = a_u1;
		u2 = a_u2;
		u3 = a_u3;
		v1 = a_v1;
		v2 = a_v2;
		v3 = a_v3;
	}

	void setNormal(const Vector3f &n1, const Vector3f &n2, const Vector3f &n3){

		m_n1 = n1;
		m_n2 = n2;
		m_n3 = n3;
		m_hasnormal = true;
	}

	

private:
	
	Vector3f m_a;
	Vector3f m_b;
	Vector3f m_c;

	Vector3f m_n1;
	Vector3f m_n2;
	Vector3f m_n3;

	
	float u1, u2, u3, v1, v2, v3;
	float abc;
	bool m_hasnormal;
	bool m_cull;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sphere : public Primitive{
public:

	Sphere(const Vector3f& centre, double radius, const Color &color);
	~Sphere();

	void hit(const Ray& ray, Hit &hit);
	void calcBounds();
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	
private:

	Vector3f m_centre;						
	double m_sqRadius, m_radius, m_rRadius;	
				
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	void hit(const Ray &ray, Hit &hit);
	void calcBounds();
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	
private:

	float distance;
	Vector3f m_normal;
	Vector3f m_u, m_v;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitive{

public:

	Torus();
	Torus(float a, float b, Color color);
	~Torus();

	void hit(const Ray &ray, Hit &hit);
	void calcBounds();
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);

private:
	float a;
	float b;
};
//////////////////////////////////////////////////////////////////////////////////////////////////




#endif