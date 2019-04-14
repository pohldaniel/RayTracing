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
	Color getColor(const Vector3f& Pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);
	std::shared_ptr<Material>  getMaterial();
	std::shared_ptr<Material>  getMaterialMesh();

	void setColor(Color color);

	bool loadObject(const char* filename, bool cull, bool smooth);
	bool loadObject(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull, bool smooth);

	bool loadObject2(const char* filename, bool cull, bool smooth);
	bool loadObject2(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull, bool smooth);

	void generateNormals();
	void generateTangents();
	void generateNormalDerivatives();

	void generateTangents2();
	void generateNormals2();

	void buildKDTree();

private:

	std::shared_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::string m_mltPath;
	std::string m_modelDirectory;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Material> m_materialMesh;

	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasNormalDerivatives;
	bool m_defaultColor;
	bool m_hasMaterials;

	int  m_numberOfVertices;
	int  m_numberOfTriangles;
	int m_numberOfMeshes;

	std::vector<float> m_vertexBuffer;
	std::vector<unsigned int> m_indexBuffer;

	std::vector<unsigned int> m_indexBufferPosition;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;
	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	std::vector<Vector2f> m_texels;

	void calcBounds();

	void indexVBO_P(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices);
	void indexVBO_PN(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices);
	void indexVBO_PT(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices);
	void indexVBO_PTN(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices);
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
	//std::shared_ptr<Texture> m_normalMap;
	std::shared_ptr<Material>  m_material;

	
	std::vector<unsigned int> m_indexBuffer;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;

	//std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	//std::vector<Vector2f> m_texels;
	std::vector<Vector4f> m_tangents;
	std::vector<Vector3f> m_bitangents;

	float	m_xmin;
	float	m_xmax;
	float	m_ymin;
	float	m_ymax;
	float	m_zmin;
	float	m_zmax;

	bool readMaterial(const char* filename);
};

#endif
