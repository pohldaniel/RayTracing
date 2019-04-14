#include "MeshTorus.h"
#include "KDTree.h"

MeshTorus::MeshTorus(float radius, float tubeRadius, bool generateNormals, bool generateTexels, bool generateTangents) : Primitive(){

	m_radius = radius;
	m_tubeRadius = tubeRadius;
	m_generateNormals = generateNormals;
	m_generateTexels = generateTexels;
	m_generateTangents = generateTangents;

	m_hasTexels = false;
	m_hasNormals = false;
	m_hasTangents = false;

	m_defaultColor = true;
	m_isInitialized = false;
	m_mainSegments = 49;
	m_tubeSegments = 49;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;

}

MeshTorus::MeshTorus(float radius, float tubeRadius) : MeshTorus(radius, tubeRadius, true, true, true){ }

MeshTorus::~MeshTorus(){}

void MeshTorus::setPrecision(int mainSegments, int tubeSegments){

	m_mainSegments = mainSegments;
	m_tubeSegments = tubeSegments;
}

void MeshTorus::buildMesh(){

	// Calculate and cache counts of vertices and indices
	int numVertices = (m_mainSegments + 1)*(m_tubeSegments + 1);
	m_primitiveRestartIndex = numVertices;
	m_numIndices = (m_mainSegments * 2 * (m_tubeSegments + 1)) + m_mainSegments - 1;

	std::vector<unsigned int> indexBuffer;
	std::vector<Vector3f> positions;
	std::vector<Vector2f> texels;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;


	float mainSegmentAngleStep = (2.0f * PI) / float(m_mainSegments);
	float tubeSegmentAngleStep = (2.0f * PI) / float(m_tubeSegments);

	float currentMainSegmentAngle = 0.0f;

	for (unsigned int i = 0; i <= m_mainSegments; i++){

		// Calculate sine and cosine of main segment angle
		float sinMainSegment = sinf(currentMainSegmentAngle);
		float cosMainSegment = cosf(currentMainSegmentAngle);
		float currentTubeSegmentAngle = 0.0f;

		for (unsigned int j = 0; j <= m_tubeSegments; j++){

			// Calculate sine and cosine of tube segment angle
			float sinTubeSegment = sinf(currentTubeSegmentAngle);
			float cosTubeSegment = cosf(currentTubeSegmentAngle);

			float x = (m_radius + m_tubeRadius * cosTubeSegment)*cosMainSegment;
			float z = (m_radius + m_tubeRadius * cosTubeSegment)*sinMainSegment;
			float y = m_tubeRadius*sinTubeSegment;

			xmin = min(x, xmin); ymin = min(y, ymin); zmin = min(z, zmin);
			xmax = max(x, xmax); ymax = max(y, ymax); zmax = max(z, zmax);

			// Calculate vertex position on the surface of torus
			Vector3f surfacePosition = Vector3f(x, y, z);

			positions.push_back(surfacePosition);
			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	if (m_generateTexels){
	
		float mainSegmentTextureStep = 1.0f / float(m_mainSegments);
		float tubeSegmentTextureStep = 1.0f / float(m_tubeSegments);

		//rotate the texture to get the same mapping like the primitive
		float currentMainSegmentTexCoordV = 0.0 - 0.25;
		for (unsigned int i = 0; i <= m_mainSegments; i++){

			//rotate the texture to get the same mapping like the primitive
			float currentTubeSegmentTexCoordU =0.0 - 0.5f ;
			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				Vector2f textureCoordinate = Vector2f( (1.0 - currentMainSegmentTexCoordV),  currentTubeSegmentTexCoordU);

				texels.push_back(textureCoordinate);
			
				currentTubeSegmentTexCoordU += tubeSegmentTextureStep;
			}

			// Update texture coordinate of main segment
			currentMainSegmentTexCoordV += mainSegmentTextureStep;
		}
	
		m_hasTexels = true;
	}

	if (m_generateNormals){

		float currentMainSegmentAngle = 0.0f;
		for (unsigned int i = 0; i <= m_mainSegments; i++){

			// Calculate sine and cosine of main segment angle
			float sinMainSegment = sin(currentMainSegmentAngle);
			float cosMainSegment = cos(currentMainSegmentAngle);

			float currentTubeSegmentAngle = 0.0f;

			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				// Calculate sine and cosine of tube segment angle
				float sinTubeSegment = sin(currentTubeSegmentAngle);
				float cosTubeSegment = cos(currentTubeSegmentAngle);

				Vector3f normal = Vector3f(cosMainSegment*cosTubeSegment,  sinTubeSegment, sinMainSegment*cosTubeSegment);
				normals.push_back(normal);

				Vector3f tangent = Vector3f(-sinTubeSegment *cosMainSegment, cosTubeSegment, -sinTubeSegment *sinMainSegment);
				tangents.push_back(tangent);

				Vector3f bitangent = Vector3f(-sinMainSegment, 0.0, cosMainSegment);
				bitangents.push_back(bitangent);

				// Update current tube angle
				currentTubeSegmentAngle += tubeSegmentAngleStep;
			}

			// Update main segment angle
			currentMainSegmentAngle += mainSegmentAngleStep;
		}

		m_hasNormals = true;
		
	}

	if (m_generateTangents){

		float currentMainSegmentAngle = 0.0f;
		for (unsigned int i = 0; i <= m_mainSegments; i++){

			// Calculate sine and cosine of main segment angle
			float sinMainSegment = sin(currentMainSegmentAngle);
			float cosMainSegment = cos(currentMainSegmentAngle);

			float currentTubeSegmentAngle = 0.0f;

			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				// Calculate sine and cosine of tube segment angle
				float sinTubeSegment = sin(currentTubeSegmentAngle);
				float cosTubeSegment = cos(currentTubeSegmentAngle);

				tangents.push_back(Vector3f(-sinTubeSegment *cosMainSegment, cosTubeSegment, -sinTubeSegment *sinMainSegment));
				bitangents.push_back(Vector3f(-sinMainSegment, 0.0, cosMainSegment));

				// Update current tube angle
				currentTubeSegmentAngle += tubeSegmentAngleStep;
			}
			// Update main segment angle
			currentMainSegmentAngle += mainSegmentAngleStep;
		}

		m_hasTangents = true;
	}

	//calculate the indices
	unsigned int currentVertexOffset = 0;
	for (unsigned int i = 0; i <= m_mainSegments; i++){

		for (unsigned int j = 0; j <= m_tubeSegments; j++){

			unsigned int vertexIndexA = currentVertexOffset;
			unsigned int vertexIndexB = currentVertexOffset + m_tubeSegments + 1;

			indexBuffer.push_back(vertexIndexA);
			indexBuffer.push_back(vertexIndexB);

			currentVertexOffset++;
		}

		// Don't restart primitive, if it's last segment, rendering ends here anyway
		if (i != m_mainSegments - 1) {
			indexBuffer.push_back(m_primitiveRestartIndex);
		}
	}

	//set up the faces
	std::shared_ptr<Triangle> triangle;
	for (unsigned int i = 0; i < indexBuffer.size() - 2; i++){

		if (indexBuffer[i] >= numVertices || indexBuffer[i + 2] >= numVertices || indexBuffer[i + 1] >= numVertices) continue;

		unsigned short j0 = i, j1 = i +1, j2 = i + 2;

		if (i & 1){

			j0 = i; j1 = i + 1; j2 = i + 2;

		}else{

			j0 = i; j1 = i + 2; j2 = i + 1;
		}


		triangle = std::shared_ptr<Triangle>(new Triangle(positions[indexBuffer[j0]], positions[indexBuffer[j1]], positions[indexBuffer[j2]], true, true));
		
		if (m_hasNormals){
			triangle->setNormal(normals[indexBuffer[j0]], normals[indexBuffer[j1]], normals[indexBuffer[j2]]);
		}
		
		if (m_hasTexels){
			triangle->setUV(texels[indexBuffer[j0]], texels[indexBuffer[j1]], texels[indexBuffer[j2]]);
		}

		if (m_hasTangents){
			triangle->setTangents(tangents[indexBuffer[j0]], tangents[indexBuffer[j1]], tangents[indexBuffer[j2]]);
			triangle->setBiTangents(bitangents[indexBuffer[j0]], bitangents[indexBuffer[j1]], bitangents[indexBuffer[j2]]);
		}
		triangle->setColor(m_color);
		triangle->m_texture = m_texture;
		triangle->m_material = m_material;

		m_triangles.push_back(triangle);
	}

	positions.clear();
	normals.clear();
	texels.clear();
	tangents.clear();
	bitangents.clear();

	calcBounds();
	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	m_isInitialized = true;
}

void MeshTorus::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	bounds = true;
}

void MeshTorus::hit(Hit &hit){
	// find the nearest intersection
	m_KDTree->intersectRec(hit);

}

void MeshTorus::setColor(Color color){
	m_color = color;
	m_defaultColor = false;
}


Color MeshTorus::getColor(const Vector3f& a_pos){

	// use one texture for the whole model
	if (m_texture){

		m_KDTree->m_primitive->m_texture = m_texture;
		return m_KDTree->m_primitive->getColor(a_pos);

	// use one texture per mesh
	// maybe the texture isn't at the path of the mlt file, then a nulltexure will created
	}else if (m_KDTree->m_primitive->m_texture && m_useTexture){

		return m_KDTree->m_primitive->getColor(a_pos);

	// use one color for the whole model
	}else if (!m_defaultColor) {
		
		return m_color;

	// use the color of every single triangle
	// the value is different for every mesh
	}else{

		return m_KDTree->m_primitive->m_color;
	}

}

std::pair<float, float> MeshTorus::getUV(const Vector3f& a_pos){

	return m_KDTree->m_primitive->getUV(a_pos);
}

Vector3f MeshTorus::getNormal(const Vector3f& a_pos){

	if (m_hasNormals){

		return (m_KDTree->m_primitive->getNormal(a_pos)).normalize();

	}

	return Vector3f(0.0, 0.0, 0.0);

}

Vector3f MeshTorus::getTangent(const Vector3f& a_pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getTangent(a_pos)).normalize();

	}

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f MeshTorus::getBiTangent(const Vector3f& a_pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getBiTangent(a_pos)).normalize();
	}

	return Vector3f(0.0, 0.0, 0.0);
}