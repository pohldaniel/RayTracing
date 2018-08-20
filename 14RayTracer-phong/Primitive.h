#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Material.h"
#include "Texture.h"
#include "Vector.h"
#include "Color.h"
#include "Hit.h"
#include "Ray.h"


class BBox{
public:
	BBox() : m_Pos(Vector3f(0, 0, 0)), m_Size(Vector3f(0, 0, 0)) {};
	BBox(Vector3f& a_Pos, Vector3f& a_Size) : m_Pos(a_Pos), m_Size(a_Size)  {};
	Vector3f& getPos() { return m_Pos; }
	Vector3f& getSize() { return m_Size; }

	inline void extend(Vector3f a){

		m_Pos = Vector3f::Min(a, m_Pos);
		m_Size = Vector3f::Max(a, m_Size);
	}

	inline float getSurfaceArea(){

		float xDim = m_Size[0] - m_Pos[0];
		float yDim = m_Size[1] - m_Pos[1];
		float zDim = m_Size[2] - m_Pos[2];

		return 2 * (xDim*yDim + yDim*zDim + zDim*xDim);
	}


	bool intersect(const Ray &ray);

	Vector3f m_Pos, m_Size;
	float m_tmin, m_tmax;
};

class Mesh;
/////////////////////////////////////////////////////////////////////////////

class Primitive {

	friend class Scene;
	friend class KDTree;

public:
	Primitive();

	Primitive(const Color &color);
	~Primitive();

	virtual void hit(const Ray& ray, Hit &hit) = 0;
	virtual void calcBounds() = 0;
	virtual Color getColor(Vector3f& a_Pos) = 0;
	virtual Vector3f getNormal(Vector3f& a_Pos) = 0;
	virtual void clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox);

	//const BBox& GetBounds();
	BBox& getBounds();

	void setTexture(Texture* texture);
	Texture* getTexture();

	void setMaterial(Material* material);
	virtual Material* getMaterial();

	Matrix4f T;
	Matrix4f invT;
	BBox box;

	Mesh *m_mesh;
	Material* m_material;
protected:
	bool bounds;
	
	Color m_color;	
	bool  orientable;
	
	Texture* m_texture;
	
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
	
	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, Color &color);
	~Triangle();

	
	void hit(const Ray& ray, Hit &hit);
	void calcBounds();
	Color getColor(Vector3f& a_Pos);
	Vector3f getNormal(Vector3f& a_Pos);

	void setUV(float a_u1, float  a_u2, float  a_u3, float a_v1, float  a_v2, float  a_v3){

		u1 = a_u1;
		u2 = a_u2;
		u3 = a_u3;
		v1 = a_v1;
		v2 = a_v2;
		v3 = a_v3;
	}

	void setNormal(Vector3f &n1, Vector3f &n2, Vector3f &n3){

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
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sphere : public Primitive{
public:

	Sphere(Vector3f& a_Centre, double a_Radius, Color color);
	~Sphere();

	void hit(const Ray& ray, Hit &hit);
	void calcBounds();
	Color getColor(Vector3f& a_Pos);
	Vector3f getNormal(Vector3f& a_Pos);
	
private:

	Vector3f m_Centre;						
	double m_SqRadius, m_Radius, m_RRadius;	
				
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	void hit(const Ray &ray, Hit &hit);
	void calcBounds();
	Color getColor(Vector3f& a_Pos);
	Vector3f getNormal(Vector3f& a_Pos);
	Vector3f m_normal;
private:

	float distance;
	
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
	Color getColor(Vector3f& a_Pos);
	Vector3f getNormal(Vector3f& a_Pos);

private:
	float a;
	float b;
};
//////////////////////////////////////////////////////////////////////////////////////////////////




#endif