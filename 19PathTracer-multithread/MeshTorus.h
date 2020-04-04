#ifndef _MESHTORUS_H
#define _MESHTORUS_H

#include "Primitive.h"

class KDTree;

class MeshTorus : public Primitive{

public:

	MeshTorus(float radius, float tubeRadius);
	MeshTorus(float radius, float tubeRadius, bool generateTexels, bool generateNormals, bool generateTangents, bool generateNormalDerivatives);
	~MeshTorus();

	void hit(Hit &hit);
	Color getColor(const Vector3f& Pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	Vector3f getNormalDu(const Vector3f& pos);
	Vector3f getNormalDv(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

	void setColor(Color color);

	void setPrecision(int mainSegments, int tubeSegments);
	void buildMesh();

	void generateNormals();
	void generateTangents();
	void generateNormalDerivatives();

private:

	std::shared_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	bool m_defaultColor;
	void calcBounds();

	int m_mainSegments;
	int m_tubeSegments;
	float m_radius;
	float m_tubeRadius;

	bool m_generateNormals;
	bool m_generateTexels;
	bool m_generateTangents;
	bool m_generateNormalDerivatives;

	bool m_isInitialized;
	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasNormalDerivatives;

	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<Vector3f> m_positions;
	std::vector<Vector2f> m_texels;
	std::vector<Vector3f> m_normals;
};


#endif
