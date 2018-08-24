#ifndef _MODEL_H
#define _MODEL_H

#include <vector>
#include <string>
#include <array>
#include <map>

#include "Primitive.h"
#include "Vector.h"


class Mesh;
class KDTree;

class Model :public OrientablePrimitive {

	
public:
	Model();
	Model(Color color);
	~Model();

	std::vector<Triangle*>	triangles;
	void hit(const Ray& a_Ray, Hit &hit);
	void calcBounds();
	Color getColor(Vector3f& a_Pos);
	Vector3f getNormal(Vector3f& a_Pos);

	bool loadObject(const char* filename);
	bool loadObject(const char* filename, Vector3f &rotate, float degree, Vector3f &translate, float scale);

	KDTree* m_KDTree;

	bool m_hasMaterial;
	std::string m_mltPath;
	std::string m_modelDirectory;
	int m_numberOfMeshes;
	std::vector<Mesh*> meshes;
	Material* getMaterial();


	std::vector<Mesh*> getMesches();
	std::string getMltPath();
	std::string getModelDirectory();
	int numberOfMeshes();
	bool hasMaterial() const;

private:
	Triangle *m_triangle;


	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;

	bool m_hasnormal;

};


class Mesh {

	friend Model;

public:

	Mesh(std::string mltName, int numberTriangles);
	Mesh(int numberTriangles);
	~Mesh();

	bool m_hasTextureCoords;
	bool m_hasNormals;
	bool m_hasTangents;

	Texture* m_texture;
	Material* m_material;

	bool readMaterial(const char* filename);



private:

	bool mltCompare(std::string* mltName);

	Color m_color;
	int m_numberTriangles;
	std::string m_mltName;

	


	float	m_xmin;
	float	m_xmax;
	float	m_ymin;
	float	m_ymax;
	float	m_zmin;
	float	m_zmax;
};

#endif
