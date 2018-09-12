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

	friend Mesh;

public:
	Model();
	Model(const Color &color);
	~Model();

	void hit(const Ray& a_Ray, Hit &hit);
	Color getColor(const Vector3f& a_Pos);
	Vector3f getNormal(const Vector3f& a_Pos);
	std::shared_ptr<Material>  getMaterial();

	bool loadObject(const char* filename, bool cull, bool smooth);
	bool loadObject(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull, bool smooth);

	void generateNormals();
	void buildKDTree();

private:

	std::unique_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::string m_mltPath;
	std::string m_modelDirectory;
	std::vector<std::shared_ptr<Mesh>> meshes;

	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasColor;
	bool m_hasMaterials;

	int  m_numberOfVertices;
	int  m_numberOfTriangles;
	int m_numberOfMeshes;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;
	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	std::vector<Vector2f> m_texels;

	void calcBounds();
};


class Mesh {

	friend Model;
	
public:

	Mesh(std::string mltName, int numberTriangles, Model *model);
	Mesh(int numberTriangles, Model *model);
	~Mesh();

	void generateNormals();

private:

	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::unique_ptr<Model> m_model;
	std::string m_mltName;
	Color m_color;

	int m_numberOfTriangles;

	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;

	std::shared_ptr<Texture> m_texture;
	std::shared_ptr<Material>  m_material;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;

	std::vector<Vector3f> m_normals;
	
	float	m_xmin;
	float	m_xmax;
	float	m_ymin;
	float	m_ymax;
	float	m_zmin;
	float	m_zmax;

	bool readMaterial(const char* filename);
};

#endif
