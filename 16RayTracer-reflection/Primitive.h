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

		m_pos = m_pos * 1.0;
		m_size = m_size * 1.01;
		
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
	friend class CompoundedObject;
	friend class Instance;
	friend class MeshTorus;

public:
	Primitive();
	virtual ~Primitive();

	virtual void hit(Hit &hit) = 0;
	virtual Vector3f getNormal(const Vector3f& pos) = 0;
	virtual Vector3f getTangent(const Vector3f& pos) = 0;
	virtual Vector3f getBiTangent(const Vector3f& pos) = 0;
	


	virtual std::pair <float, float> getUV(const Vector3f& a_pos) = 0;

	virtual BBox& getBounds();
	virtual void setTexture(Texture* texture);
	virtual std::shared_ptr<Texture> getTexture();
	virtual void setMaterial(Material* material);
	virtual std::shared_ptr<Material> getMaterial();
	virtual void setColor(Color color);
	virtual Color getColor(const Vector3f& pos);
	
	virtual Vector3f getNormalDu(const Vector3f& pos);
	virtual Vector3f getNormalDv(const Vector3f& pos);

protected:

	bool bounds;	
	bool m_useTexture;
	
	BBox box;

	std::shared_ptr<Material> m_material;
	std::shared_ptr<Texture> m_texture;
	Color m_color;

	void clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox);
	virtual void calcBounds() = 0;
	
};
/////////////////////////////////////////////////////////////////////////////
class Instance : public Primitive {

public:

	Instance(Primitive *primitive);
	Instance::~Instance();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);
	std::shared_ptr<Texture> getTexture();
	std::shared_ptr<Material> getMaterial();
	Color getColor(const Vector3f& pos);
	BBox& getBounds();

	void setColor(Color color);
	
	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float);

private:
	Matrix4f T;
	Matrix4f invT;

	std::shared_ptr<Primitive> m_primitive;

	void calcBounds();
	bool m_defaultColor;
};

/////////////////////////////////////////////////////////////////////////////
class CompoundedObject : public Primitive {

	public:

	using Primitive::setMaterial;
	using Primitive::setTexture;
	using Primitive::setColor;

	CompoundedObject();
	~CompoundedObject();

	void hit(Hit &hit);
	Color getColor(const Vector3f& pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

	void setColorAll(const Color& color);
	void setTextureAll(Texture* texture);
	std::shared_ptr<Texture> getTexture();
	void setMaterialAll(Material* material);
	std::shared_ptr<Material> getMaterial();
	void addPrimitive(Primitive* primitive);
	void setSeperate(bool seperate);

protected:

	std::vector<std::shared_ptr<Primitive>>	m_primitives;

private:

	
	void calcBounds();

	std::shared_ptr<Primitive> m_primitive;
	bool m_seperate;
	
};
//////////////////////////////////////////////////////////////////////////////
class Triangle :public Primitive{

	friend class Mesh;

public:
	
	Triangle(const Vector3f &a_V1, const Vector3f &a_V2, const Vector3f &a_V3, const bool cull, const bool smooth);
	~Triangle();

	void hit(Hit &hit);
	Color getColor(const Vector3f& a_pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

	Vector3f getNormalDu(const Vector3f& pos);
	Vector3f getNormalDv(const Vector3f& pos);

	void setUV(const Vector2f &uv1, const Vector2f &uv2, const Vector2f &uv3){

		m_uv1 = uv1; m_uv2 = uv2; m_uv3 = uv3;
		m_hasTextureCoords = true;
	}

	void setNormal(const Vector3f &n1, const Vector3f &n2, const Vector3f &n3){

		m_n1 = n1; m_n2 = n2; m_n3 = n3;
		m_hasNormals = true;
		
	}

	
	void setTangents(const Vector3f &t1, const Vector3f &t2, const Vector3f &t3){
		
		m_t1 = t1; m_t2 = t2; m_t3 = t3;
		m_hasTangents = true;
	}

	void setBiTangents(const Vector3f &bt1, const Vector3f &bt2, const Vector3f &bt3){

		m_bt1 = bt1; m_bt2 = bt2; m_bt3 = bt3;
	}

	void setNormalDu(const Vector3f &nDu1, const Vector3f &nDu2, const Vector3f &nDu3){

		m_nDu1 = nDu1; m_nDu2 = nDu2; m_nDu3 = nDu3;
		m_hasNormalDerivatives = true;
	}

	void setNormalDv(const Vector3f &nDv1, const Vector3f &nDv2, const Vector3f &nDv3){

		m_nDv1 = nDv1; m_nDu2 = nDv2; m_nDu3 = nDv3;
	}


private:

	Vector3f m_a, m_b, m_c;
	Vector3f m_edge1, m_edge2;
	Vector2f m_uv1, m_uv2, m_uv3;
	
	//flat shading
	Vector3f m_normal;

	//smooth shading
	Vector3f m_n1, m_n2, m_n3;
	Vector3f m_t1, m_t2, m_t3;
	Vector3f m_bt1, m_bt2, m_bt3;
	Vector3f m_nDu1, m_nDu2, m_nDu3;
	Vector3f m_nDv1, m_nDv2, m_nDv3;

	float abc;
	
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasNormalDerivatives;
	bool m_hasTextureCoords;
	bool m_cull;
	bool m_smooth;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance);
	~Plane();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	float distance;
	Vector3f m_normal;
	Vector3f m_u, m_v;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Disk : public Primitive{

public:

	Disk();
	Disk(Vector3f center, Vector3f normal, float radius);
	~Disk();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	Vector3f m_center;
	Vector3f m_normal;
	float m_radius;
	float m_sqRadius;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Annulus : public Primitive{

public:

	Annulus();
	Annulus(Vector3f center, Vector3f normal, float outerRadius, float innerRadius);
	~Annulus();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& pos);

private:

	Vector3f m_center;
	Vector3f m_normal;
	float m_outerRadius;
	float m_innerRadius;
	float m_sqOuterRadius;
	float m_sqInnerRadius;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Sphere : public Primitive{
public:

	Sphere();
	Sphere(const Vector3f& centre, float radius);
	~Sphere();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& pos);

	Vector3f getNormalDu(const Vector3f& pos);
	Vector3f getNormalDv(const Vector3f& pos);

private:

	Vector3f m_centre;
	float m_sqRadius, m_radius, m_invRadius;

	void calcBounds();

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Torus : public Primitive{

public:

	Torus();
	Torus(float m_radius, float m_tubeRadius);
	~Torus();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& a_pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:
	float m_radius;
	float m_tubeRadius;

	void calcBounds();
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Cube : public Primitive{

	enum Sides { Left, Right, Top, Bottom, Front, Back, None };

public:
	Cube();
	Cube(Vector3f pos, Vector3f size);
	~Cube();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	void calcBounds();

	Vector3f m_pos;
	Vector3f m_size;
	Sides side = Sides::None;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
namespace primitive{

	class Rectangle : public Primitive{

	public:
		Rectangle();
		Rectangle(Vector3f pos, Vector3f a, Vector3f b);
		~Rectangle();

		void hit(Hit &hit);
		Vector3f getNormal(const Vector3f& pos);
		Vector3f getTangent(const Vector3f& pos);
		Vector3f getBiTangent(const Vector3f& pos);
		std::pair <float, float> getUV(const Vector3f& a_pos);

		void flipNormal();

	private:

		void calcBounds();

		Vector3f		m_pos;   			// corner vertex 
		Vector3f		m_a;				// side
		Vector3f		m_b;				// side
		float           m_lenA;
		float           m_lenB;
		float			m_sqA;				// square of the length of side a
		float			m_sqB;				// square of the length of side b

		float			m_area;			// for rectangular lights
		float			m_invArea;		// for rectangular lights

		Vector3f m_normal;
		Vector3f m_v;
		float    m_lenV;
	};
}
//////////////////////////////////////////////////////////////////////////////////////////////////
class OpenCylinder : public Primitive{

public:

	OpenCylinder();
	OpenCylinder(float bottom, float top, float radius);
	~OpenCylinder();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	void calcBounds();

	double		m_bottom;			// bottom y value
	double		m_top;				// top y value
	double		m_radius;			// radius
	double		m_invRadius;  		// one over the radius	
	Vector3f    m_normal;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class OpenCone : public Primitive{

public:

	OpenCone();
	OpenCone(float radius, float height);
	~OpenCone();

	void hit(Hit &hit);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

private:

	void calcBounds();

	double m_height;
	double m_radius;
	double m_tanSq;  // tan of cone angle
	Vector3f m_center;	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class SolidCylinder : public CompoundedObject{

public:

	using::Primitive::setTexture;
	using::Primitive::setColor;
	using::Primitive::setMaterial;

	enum Components {BottomDisk, Wall, TopDisk };

	SolidCylinder();
	SolidCylinder(float bottom, float top, float radius);
	~SolidCylinder();

	void hit(Hit &hit);

	void setColor(Color color, Components components);
	void setMaterial(Material* material, Components components);
	void setTexture(Texture* texture, Components components);

private:

	double		m_bottom;			// bottom y value
	double		m_top;				// top y value
	double		m_radius;			// radius
	double		m_invRadius;  		// one over the radius	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class SolidCone : public CompoundedObject{

public:

	using::Primitive::setTexture;
	using::Primitive::setColor;
	using::Primitive::setMaterial;

	enum Components { BottomDisk, Wall};

	SolidCone();
	SolidCone(float radius, float top);
	~SolidCone();

	void hit(Hit &hit);

	void setColor(Color color, Components components);
	void setMaterial(Material* material, Components components);
	void setTexture(Texture* texture, Components components);

private:

	double m_top;
	double m_radius;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Box : public CompoundedObject{

public:

	using::Primitive::setTexture;
	using::Primitive::setColor;
	using::Primitive::setMaterial;

	enum Components { FrontFace, BackFace, TopFace, BottomFace, RightFace, LeftFace };

	Box();
	Box(const Vector3f& pos, const Vector3f& size);
	~Box();

	void hit(Hit &hit);

	void setColor(Color color, Components components);
	void setMaterial(Material* material, Components components);
	void setTexture(Texture* texture, Components components);

	void flipNormals();

private:

	Vector3f m_pos;
	Vector3f m_size;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class BeveledCylinder : public CompoundedObject{

public:
	
	using::Primitive::setTexture;
	using::Primitive::setColor;
	using::Primitive::setMaterial;

	enum Components { BottomDisk, BottomTorus, Wall, TopTorus, TopDisk };

	BeveledCylinder();
	BeveledCylinder(float bottom, float top, float radius, float bevelRadius);
	~BeveledCylinder();

	void hit(Hit &hit);

	void setColor(Color color, Components components);
	void setMaterial(Material* material, Components components);
	void setTexture(Texture* texture, Components components);

private:

	double		m_bottom;			// bottom y value
	double		m_top;				// top y value
	double		m_radius;			// radius
	double		m_invRadius;  		// one over the radius	
};

#endif