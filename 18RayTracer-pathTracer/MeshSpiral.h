#ifndef _MESHSPIRAL_H
#define _MESHSPIRAL_H

#include "Primitive.h"

class KDTree;

class MeshSpiral : public Primitive{

public:

	MeshSpiral(float radius, float tubeRadius, int numRotations, float length);
	MeshSpiral(float radius, float tubeRadius, int numRotations, float length, bool repeatTexture, bool generateTexels, bool generateNormals, bool generateTangents);
	~MeshSpiral();

	void hit(Hit &hit);
	Color getColor(const Vector3f& Pos);
	Vector3f getNormal(const Vector3f& pos);
	Vector3f getTangent(const Vector3f& pos);
	Vector3f getBiTangent(const Vector3f& pos);
	Vector3f getNormalDu(const Vector3f& pos);
	Vector3f getNormalDv(const Vector3f& pos);
	std::pair <float, float> getUV(const Vector3f& a_pos);

	void setColor(Color color);
	void repeatTexture(bool repeatTexture);
	void setPrecision(int mainSegments, int tubeSegments);
	void buildMesh();

	void generateNormals();

private:

	int m_numRotations;
	float m_length;
	bool m_repeatTexture;

	int m_mainSegments;
	int m_tubeSegments;
	float m_radius;
	float m_tubeRadius;

	std::shared_ptr<KDTree> m_KDTree;
	std::vector<std::shared_ptr<Triangle>>	m_triangles;
	bool m_defaultColor;
	void calcBounds();

	bool m_generateNormals;
	bool m_generateTexels;
	bool m_generateTangents;
	bool m_generateNormalDerivatives;

	bool m_isInitialized;
	bool m_hasTexels;
	bool m_hasNormals;
	bool m_hasTangents;
	bool m_hasNormalDerivatives;

	int m_numIndices = 0;
	int m_primitiveRestartIndex = 0;

	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<Vector3f> m_positions;
};


#endif