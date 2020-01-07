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

class Model :public Primitive {

	friend Mesh;

public:
	Model();
	~Model();

	void hit(Hit &hit);
	bool shadowHit(Ray &ray);
	Color getColor(const Vector3f& Pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	Vector3f getNormalDu(const Vector3f& pos);
	Vector3f getNormalDv(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);
	std::shared_ptr<Material>  getMaterial();
	std::shared_ptr<Material>  getMaterialMesh();

	void setColor(Color color);

	bool loadObject(const char* filename, bool cull, bool smooth);
	bool loadObject(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull, bool smooth);

	void generateNormals();
	void generateTangents();
	void generateNormalDerivatives();

	void buildKDTree();

private:

	std::shared_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::string m_mltPath;
	std::string m_modelDirectory;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Material> m_materialMesh;

	float	xmin, xmax, ymin, ymax, zmin, zmax;
	
	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasNormalDerivatives;
	bool m_defaultColor;
	bool m_hasMaterials;

	int  m_numberOfVertices;
	int  m_numberOfTriangles;
	int m_numberOfMeshes;

	std::vector<unsigned int> m_indexBufferPosition;
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
	void generateTangents();

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

	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	std::vector<Vector2f> m_texels;
	

	float	m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;

	bool readMaterial(const char* filename);
};

#endif
