#ifndef _MODEL_H
#define _MODEL_H

#include <vector>
#include <string>
#include <array>
#include <map>
#include <memory>

#include "Primitive.h"
#include "Vector.h"


class Mesh;
class KDTree;

class Model :public OrientablePrimitive {

	
public:
	Model();
	Model(const Color &color);
	~Model();

	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	void hit(const Ray& a_Ray, Hit &hit);
	void calcBounds();
	Color getColor(const Vector3f& a_Pos);
	Vector3f getNormal(const Vector3f& a_Pos);

	bool loadObject(const char* filename, bool cull);
	bool loadObject(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull);

	std::unique_ptr<KDTree> m_KDTree;

	bool m_hasMaterial;
	std::string m_mltPath;
	std::string m_modelDirectory;
	int m_numberOfMeshes;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Material>  getMaterial();

	void generateNormals();
	void generateTangents();
	void buildKDTree();

private:
	
	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	bool m_hasNormals;
	bool m_hasTangents;
};


class Mesh {

	friend Model;
	
public:

	Mesh(std::string mltName, int numberTriangles);
	Mesh(int numberTriangles);
	~Mesh();

	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;

	std::shared_ptr<Texture> m_texture;
	std::shared_ptr<Material>  m_material;

	bool readMaterial(const char* filename);

	void generateNormals();
	void generateTangents();

private:

	
	std::string m_mltName;
	Color m_color;
	int m_numberTriangles;
	int m_numberOfVertices;
	int m_numberOfTexels;
	int m_numberOfNormals;

	std::vector<std::shared_ptr<Triangle>>	m_triangles;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;

	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	std::vector<Vector2f> m_texels;
	std::vector<Vector4f> m_tangents;
	std::vector<Vector3f> m_bitangents;

	float	m_xmin;
	float	m_xmax;
	float	m_ymin;
	float	m_ymax;
	float	m_zmin;
	float	m_zmax;
};

#endif
