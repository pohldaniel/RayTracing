#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Vector.h"
#include "Color.h"
#include "Hit.h"
#include "Ray.h"


class BBox{
public:
	BBox() : m_Pos(Vector3f(0, 0, 0)), m_Size(Vector3f(0, 0, 0)) {};
	BBox(Vector3f& a_Pos, Vector3f& a_Size) : m_Pos(a_Pos), m_Size(a_Size)  {};
	Vector3f& GetPos() { return m_Pos; }
	Vector3f& GetSize() { return m_Size; }
	bool intersect(BBox& b2)
	{
		Vector3f v1 = b2.GetPos(), v2 = b2.GetPos() + b2.GetSize();
		Vector3f v3 = m_Pos, v4 = m_Pos + m_Size;
		return ((v4[0] >= v1[0]) && (v3[0] <= v2[0]) && // x-axis overlap
			(v4[1] >= v1[1]) && (v3[1] <= v2[1]) && // y-axis overlap
			(v4[2] >= v1[2]) && (v3[2] <= v2[2]));   // z-axis overlap
	}

	bool intersect(const Ray &ray);

	bool Contains(Vector3f a_Pos)
	{
		Vector3f v1 = m_Pos, v2 = m_Pos + m_Size;
		return ((a_Pos[0] >= v1[0]) && (a_Pos[0] <= v2[0]) &&
			(a_Pos[1] >= v1[1]) && (a_Pos[1] <= v2[1]) &&
			(a_Pos[2] >= v1[2]) && (a_Pos[2] <= v2[2]));
	}
	double w() { return m_Size[0]; }
	double h() { return m_Size[1]; }
	double d() { return m_Size[2]; }
	double x() { return m_Pos[0]; }
	double y() { return m_Pos[1]; }
	double z() { return m_Pos[2]; }
private:
	Vector3f m_Pos, m_Size;
	
};


/////////////////////////////////////////////////////////////////////////////

class Primitive {

	friend class Scene;
public:
	Primitive();

	Primitive(const Color &color, const Vector3f &normal);
	~Primitive();

	virtual void hit(const Ray& ray, Hit &hit) = 0;
	

protected:
	Color color;
	Vector3f normal;	
	bool  orientable;
	Matrix4f T;
};
/////////////////////////////////////////////////////////////////////////////
class OrientablePrimitive : public Primitive {


public:

	OrientablePrimitive();
	OrientablePrimitive(const Color& color, const Vector3f &normal);
	~OrientablePrimitive();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float);

};

/////////////////////////////////////////////////////////////////////////////

class Triangle : public Primitive
{
public:
	
	Triangle();

	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, Vector3f &normal);
	~Triangle();

	
	void hit(const Ray& ray, Hit &hit);
	bool IntersectBox(BBox& a_Box);
	void CalculateRange(double& a_Pos1, double& a_Pos2, int a_Axis);
	
	
	// triangle-box intersection stuff
	bool PlaneBoxOverlap(Vector3f& a_Normal, Vector3f& a_Vert, Vector3f& a_MaxBox);
	bool IntersectTriBox(Vector3f& a_BoxCentre, Vector3f& a_BoxHalfsize, Vector3f& a_V0, Vector3f& a_V1, Vector3f& a_V2);

	// triangle primitive methods
	Vector3f GetPos(int a_Idx);
	

private:
	
	Vector3f m_a;
	Vector3f m_b;
	Vector3f m_c;

	double m_U, m_V;						
	double nu, nv, nd;				
	int k;								
	double bnu, bnv;				
	double cnu, cnv;		

};

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sphere : public Primitive{
public:
	Sphere();
	Sphere(Vector3f& a_Centre, double a_Radius, Color color);
	~Sphere();

	Vector3f GetNormal(Vector3f& a_Pos);
	void hit(const Ray& ray, Hit &hit);
	bool IntersectBox(BBox& a_Box);
	void CalculateRange(double& a_Pos1, double& a_Pos2, int a_Axis);

	bool IntersectSphereBox(Vector3f& a_Centre, BBox& a_Box);
	

private:

	Vector3f m_Centre;						
	double m_SqRadius, m_Radius, m_RRadius;	
	Vector3f m_Ve, m_Vn, m_Vc;				
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	void hit(const Ray &ray, Hit &hit);



private:

	float distance;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitive{

public:

	Torus();
	Torus(float a, float b, Color color);
	~Torus();

	void hit(const Ray &ray, Hit &hit);

private:
	float a;
	float b;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class KdTreeNode;
class KdTree;
struct kdstack
{
	KdTreeNode* node;
	double t;
	Vector3f pb;
	int prev, dummy1, dummy2;
};

//class Mesh;

class Mesh :public OrientablePrimitive {


public:
	Mesh();
	~Mesh();

	std::vector<Triangle*>	triangles;
	int traverse(const Ray& a_Ray, Hit &hit);
	void hit(const Ray& a_Ray, Hit &hit);

	BBox m_Extends;
	KdTree* m_KdTree;

	int* m_Mod;
	kdstack* m_Stack;

	bool loadObject(const char* filename);
	bool loadObject(const char* filename , float scale);
	
};



#endif