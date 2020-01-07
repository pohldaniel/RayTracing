#ifndef _MODELINDEXED_H
#define _MODELINDEXED_H

#include <vector>
#include <string>
#include <array>
#include <map>
#include <memory>

#include "Primitive.h"
#include "Vector.h"


class MeshIndexed;
class KDTree;

class ModelIndexed : public Primitive {

public:
	ModelIndexed();
	~ModelIndexed();

	void hit(Hit &hit);
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

	void generateTangents();
	void generateNormals();
	void generateNormalDerivatives();

	void buildKDTree();

private:

	std::shared_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::string m_mltPath;
	std::string m_modelDirectory;
	std::vector<std::shared_ptr<MeshIndexed>> meshes;
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

	std::vector<float> m_vertexBuffer;
	std::vector<unsigned int> m_indexBuffer;
	std::map<int, std::vector<int>> m_vertexCache;

	/*std::vector<unsigned int> m_indexBufferPosition;
	std::vector<unsigned int> m_indexBufferTexel;
	std::vector<unsigned int> m_indexBufferNormal;
	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_normals;
	std::vector<Vector2f> m_texels;*/

	void calcBounds();

	int addVertex(int hash, float *pVertex, int stride);

};

class MeshIndexed {

	friend ModelIndexed;

public:

	MeshIndexed(std::string mltName, int numberTriangles, ModelIndexed *model);
	MeshIndexed(int numberTriangles, ModelIndexed *model);
	~MeshIndexed();

private:

	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	std::unique_ptr<ModelIndexed> m_model;
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

	float	m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;

	bool readMaterial(const char* filename);
};

#endif