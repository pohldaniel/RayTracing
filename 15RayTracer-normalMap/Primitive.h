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
	BBox(const Vector3f& a_pos, const Vector3f& a_size) : m_pos(a_pos), m_size(a_size)  {};
	Vector3f& getPos() { return m_pos; }
	Vector3f& getSize() { return m_size; }

	inline void doubleSize(){

		m_pos = m_pos *2;
		m_size = m_size * 2;
		
	}

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
	virtual Color getColor(const Vector3f& a_pos) = 0;
	virtual Vector3f getNormal(const Vector3f& a_pos) = 0;
	virtual Vector3f getTangent(const Vector3f& a_pos) = 0;
	virtual Vector3f getBiTangent(const Vector3f& a_pos) = 0;
	virtual std::pair <float, float> getUV(const Vector3f& a_pos) = 0;

	BBox& getBounds();

	void setTexture(Texture* texture);
	std::shared_ptr<Texture> getTexture();
	
	void setMaterial(Material* material);
	virtual std::shared_ptr<Material> getMaterial();

	

protected:

	bool orientable;
	bool bounds;	
	bool m_useTexture;
	
	Matrix4f T;
	Matrix4f invT;
	BBox box;

	std::shared_ptr<Material> m_material;
	std::shared_ptr<Texture> m_texture;
	Color m_color;

	void clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox);
	virtual void calcBounds() = 0;
	
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

	friend class Mesh;

public:
	
	Triangle(const Vector3f &a_V1, const Vector3f &a_V2, const Vector3f &a_V3, const Color &color, const bool cull, const bool smooth);
	~Triangle();

	void hit(const Ray& ray, Hit &hit);
	Color getColor(const Vector3f& a_pos);
	Vector3f getNormal(const Vector3f& a_pos);
	Vector3f getTangent(const Vector3f& a_pos);
	Vector3f getBiTangent(const Vector3f& a_pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);


	void setUV(const Vector2f &uv1, const Vector2f &uv2, const Vector2f &uv3){

		m_uv1 = uv1;
		m_uv2 = uv2;
		m_uv3 = uv3;
		m_hasTextureCoords = true;
	}

	void setNormal(const Vector3f &n1, const Vector3f &n2, const Vector3f &n3){

		m_n1 = n1;
		m_n2 = n2;
		m_n3 = n3;
		m_hasNormals = true;
		
	}

	
	void setTangents(const Vector4f &t1, const Vector4f &t2, const Vector4f &t3){
		
		m_t1 = t1;
		m_t2 = t2;
		m_t3 = t3;
		m_hasTangents = true;
	}

	void setBiTangents(const Vector3f &bt1, const Vector3f &bt2, const Vector3f &bt3){

		m_bt1 = bt1;
		m_bt2 = bt2;
		m_bt3 = bt3;
		//m_hasBiTangents = true;
	}


	
private:

	Vector3f m_a;
	Vector3f m_b;
	Vector3f m_c;

	Vector3f m_edge1;
	Vector3f m_edge2;

	Vector2f m_uv1;
	Vector2f m_uv2;
	Vector2f m_uv3;

	Vector3f m_normal;

	Vector3f m_n1;
	Vector3f m_n2;
	Vector3f m_n3;

	Vector3f m_t1;
	Vector3f m_t2;
	Vector3f m_t3;

	Vector3f m_bt1;
	Vector3f m_bt2;
	Vector3f m_bt3;

	float abc;
	
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasTextureCoords;
	bool m_cull;
	bool m_smooth;

	void calcBounds();
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sphere : public OrientablePrimitive{
public:

	Sphere(const Vector3f& centre, float radius, const Color &color);
	~Sphere();

	void hit(const Ray& ray, Hit &hit);
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& a_pos);
	Vector3f getBiTangent(const Vector3f& a_pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	Vector3f m_centre;						
	float m_sqRadius, m_radius, m_rRadius;	

	void calcBounds();
				
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	void hit(const Ray &ray, Hit &hit);
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& a_pos);
	Vector3f getBiTangent(const Vector3f& a_pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	float distance;
	Vector3f m_normal;
	Vector3f m_u, m_v;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitive{

public:

	Torus();
	Torus(float a, float b, Color color);
	~Torus();

	void hit(const Ray &ray, Hit &hit);
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& a_pos);
	Vector3f getBiTangent(const Vector3f& a_pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:
	float a;
	float b;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////




#endif