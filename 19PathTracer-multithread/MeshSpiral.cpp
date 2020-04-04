#include "MeshSpiral.h"
#include "KDTree.h"

MeshSpiral::MeshSpiral(float radius, float tubeRadius, int numRotations, float length, bool repeatTexture, bool generateTexels, bool generateNormals, bool generateTangents) : Primitive(){

	m_radius = radius;
	m_tubeRadius = tubeRadius;
	m_numRotations = numRotations;
	m_length = length;
	m_repeatTexture = repeatTexture;

	m_generateNormals = generateNormals;
	m_generateTexels = generateTexels;
	m_generateTangents = generateTangents;


	m_hasTexels = false;
	m_hasNormals = false;
	m_hasTangents = false;
	m_hasNormalDerivatives = false;

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

MeshSpiral::MeshSpiral(float radius, float tubeRadius, int numRotations, float length) : MeshSpiral(radius, tubeRadius, numRotations, length, false, true, true, true){ }

MeshSpiral::~MeshSpiral(){}

void MeshSpiral::setPrecision(int mainSegments, int tubeSegments){

	m_mainSegments = mainSegments;
	m_tubeSegments = tubeSegments;
}

void MeshSpiral::buildMesh(){

	if (m_isInitialized) return;

	std::vector<Vector2f> texels;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;

	float mainSegmentAngleStep = (2.0f * PI) / float(m_mainSegments);
	float tubeSegmentAngleStep = (2.0f * PI) / float(m_tubeSegments);
	float pitch = m_length / m_numRotations;

	float currentMainSegmentAngle = 0.0f;

	for (unsigned int i = 0; i <= m_mainSegments * m_numRotations; i++){

		// Calculate sine and cosine of main segment angle
		float sinMainSegment = sinf(currentMainSegmentAngle);
		float cosMainSegment = cosf(currentMainSegmentAngle);

		float currentTubeSegmentAngle = 0.0f;

		if (i > 0 && (m_mainSegments + 1) % i == 0) currentTubeSegmentAngle = 0.0f;

		for (unsigned int j = 0; j <= m_tubeSegments; j++){

			// Calculate sine and cosine of tube segment angle
			float sinTubeSegment = sinf(currentTubeSegmentAngle);
			float cosTubeSegment = cosf(currentTubeSegmentAngle);

			//u = mainSegmentAngle	v = tubeSegmentAngle	R = m_radius	r = m_tubeRadius

			//	[x] = [(R + r cos(u)) cos(v))
			//	[y] = [h * v/(2PI) + rsin(u)]
			//	[z] = [(R + r cos(u)) sin(v)]
			float x = (m_radius + m_tubeRadius * cosTubeSegment)*cosMainSegment;
			float z = (m_radius + m_tubeRadius * cosTubeSegment)*sinMainSegment;
			float y = pitch * (currentMainSegmentAngle * invTWO_PI) + m_tubeRadius * sinTubeSegment;

			xmin = min(x, xmin); ymin = min(y, ymin); zmin = min(z, zmin);
			xmax = max(x, xmax); ymax = max(y, ymax); zmax = max(z, zmax);

			Vector3f surfacePosition = Vector3f(x, y, z);
			m_positions.push_back(surfacePosition);

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	if (m_generateNormals){

		float currentMainSegmentAngle = 0.0f;
		for (unsigned int i = 0; i <= m_mainSegments * m_numRotations; i++){

			// Calculate sine and cosine of main segment angle
			float sinMainSegment = sin(currentMainSegmentAngle);
			float cosMainSegment = cos(currentMainSegmentAngle);

			float currentTubeSegmentAngle = 0.0f;

			if (i > 0 && (m_mainSegments + 1) % i == 0) currentTubeSegmentAngle = 0.0f;

			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				// Calculate sine and cosine of tube segment angle
				float sinTubeSegment = sin(currentTubeSegmentAngle);
				float cosTubeSegment = cos(currentTubeSegmentAngle);

				//u = mainSegmentAngle	v = tubeSegmentAngle	R = m_radius	r = m_tubeRadius
				//dp/du x dp/dv

				//	[n1] = [(Rr + r^2 cos(u)) cos(v) cos(u) + r sin(u)sin(v)h/(2PI) )
				//	[n2] = [(Rr + r^2 cos(u)) sin(u)]
				//	[n3] = [(Rr + r^2 cos(u)) sin(v) cos(u) - r sin(u)cos(v)h/(2PI)]

				float tmp = m_radius * m_tubeRadius + m_tubeRadius * m_tubeRadius * cosTubeSegment;
				float n1 = tmp * cosMainSegment * cosTubeSegment + m_tubeRadius * pitch * invTWO_PI * sinMainSegment * sinTubeSegment;
				float n2 = tmp * sinTubeSegment;
				float n3 = tmp * sinMainSegment * cosTubeSegment - m_tubeRadius * pitch * invTWO_PI * cosMainSegment * sinTubeSegment;

				Vector3f normal = Vector3f(n1, n2, n3).normalize();
				normals.push_back(normal);

				// Update current tube angle
				currentTubeSegmentAngle += tubeSegmentAngleStep;
			}

			// Update main segment angle
			currentMainSegmentAngle += mainSegmentAngleStep;
		}

		m_hasNormals = true;
	}

	if (m_generateTexels && m_numRotations > 0){

		int foo = (m_repeatTexture) ? m_mainSegments : m_mainSegments * m_numRotations;
		int offset = (m_repeatTexture) ? m_numRotations - 1 : 0;

		float mainSegmentTextureStep = 1.0 / (float(foo));
		float tubeSegmentTextureStep = 1.0f / float(m_tubeSegments);

		//rotate the texture to like the meshTorus
		float currentMainSegmentTexCoordU = 0.0 ;

		for (unsigned int i = 0; i <= m_mainSegments * m_numRotations + offset; i++){

			if (m_repeatTexture && i > 0 && i % (m_mainSegments + 1) == 0) {
				
				currentMainSegmentTexCoordU = 0.0;
			}

			//rotate the texture to like the meshTorus
			float currentTubeSegmentTexCoordV = 0.0 - 0.5f;
			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				Vector2f textureCoordinate = Vector2f((1.0 - currentMainSegmentTexCoordU), currentTubeSegmentTexCoordV);

				texels.push_back(textureCoordinate);

				currentTubeSegmentTexCoordV += tubeSegmentTextureStep;
			}

			// Update texture coordinate of main segment
			currentMainSegmentTexCoordU += mainSegmentTextureStep;
		}

		m_hasTexels = true;
	}



	if (m_generateTangents){
	
	
		float currentMainSegmentAngle = 0.0f;
		for (unsigned int i = 0; i <= m_mainSegments * m_numRotations; i++){

			// Calculate sine and cosine of main segment angle
			float sinMainSegment = sin(currentMainSegmentAngle);
			float cosMainSegment = cos(currentMainSegmentAngle);

			float currentTubeSegmentAngle = 0.0f;

			if (i > 0 && (m_mainSegments + 1) % i == 0) currentTubeSegmentAngle = 0.0f;

			for (unsigned int j = 0; j <= m_tubeSegments; j++){

				// Calculate sine and cosine of tube segment angle
				float sinTubeSegment = sin(currentTubeSegmentAngle);
				float cosTubeSegment = cos(currentTubeSegmentAngle);

				//u = mainSegmentAngle	v = tubeSegmentAngle	R = m_radius	r = m_tubeRadius
				//dp/du 

				//	[t1] = [-rsin(u)cos(v) )
				//	[t2] = [rcos(u)]
				//	[t3] = [-rsin(u)sins(v)]
				Vector3f tangent = Vector3f(-sinTubeSegment*cosMainSegment, cosTubeSegment, -sinTubeSegment*sinMainSegment);
				tangents.push_back(tangent);

				//u = mainSegmentAngle	v = tubeSegmentAngle	R = m_radius	r = m_tubeRadius
				//dp/dv 

				//	[bt1] = [-(R + rcos(u))sin(v)]
				//	[bt2] = [h/(2PI)]
				//	[bt3] = [(R + rcos(u))cos(v)]

				float bt1 = -(m_radius + m_tubeRadius*cosTubeSegment)*sinMainSegment;
				float bt2 = pitch * invTWO_PI;
				float bt3 = (m_radius + m_tubeRadius*cosTubeSegment)*cosMainSegment;

				Vector3f bitangent = Vector3f(bt1, bt2, bt3).normalize();
				bitangents.push_back(bitangent);

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
	for (unsigned int i = 0; i < m_mainSegments * m_numRotations; i++){

		for (unsigned int j = 0; j < m_tubeSegments; j++){

			unsigned int vertexIndexA = currentVertexOffset;
			unsigned int vertexIndexB = currentVertexOffset + m_tubeSegments + 1;


			unsigned int vertexIndexD = currentVertexOffset + 1;
			unsigned int vertexIndexE = currentVertexOffset + m_tubeSegments + 1;

			unsigned int vertexIndexC;
			unsigned int vertexIndexF;

			if ((currentVertexOffset + 1) == m_tubeSegments){

				vertexIndexC = currentVertexOffset + 1 - m_tubeSegments;
				vertexIndexF = currentVertexOffset + m_tubeSegments + 2 - m_tubeSegments;

			}else{
				vertexIndexC = currentVertexOffset + 1;
				vertexIndexF = currentVertexOffset + m_tubeSegments + 2;
			}

			m_indexBuffer.push_back(vertexIndexA); m_indexBuffer.push_back(vertexIndexB); m_indexBuffer.push_back(vertexIndexC);
			m_indexBuffer.push_back(vertexIndexD); m_indexBuffer.push_back(vertexIndexE); m_indexBuffer.push_back(vertexIndexF);

			currentVertexOffset++;
		}

		currentVertexOffset++;
	}

	//set up the faces
	std::shared_ptr<Triangle> triangle;
	for (unsigned int i = 0; i < m_indexBuffer.size(); i = i + 3){

		triangle = std::shared_ptr<Triangle>(new Triangle(m_positions[m_indexBuffer[i]], m_positions[m_indexBuffer[i + 1]], m_positions[m_indexBuffer[i + 2]], true, true));


		if (m_hasNormals){
			triangle->setNormal(normals[m_indexBuffer[i]], normals[m_indexBuffer[i + 1]], normals[m_indexBuffer[i + 2]]);
		}

		if (m_hasTexels){
			triangle->setUV(texels[m_indexBuffer[i]], texels[m_indexBuffer[i + 1]], texels[m_indexBuffer[i + 2]]);
		}

		if (m_hasTangents){
			triangle->setTangents(tangents[m_indexBuffer[i]], tangents[m_indexBuffer[i + 1]], tangents[m_indexBuffer[i + 2]]);
			triangle->setBiTangents(bitangents[m_indexBuffer[i]], bitangents[m_indexBuffer[i + 1]], bitangents[m_indexBuffer[i + 2]]);
		}

		triangle->setColor(m_color);
		triangle->m_texture = m_texture;
		triangle->m_material = m_material;

		m_triangles.push_back(triangle);
	}

	
	normals.clear();
	texels.clear();
	tangents.clear();
	bitangents.clear();
	

	calcBounds();
	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	m_isInitialized = true;
}

void MeshSpiral::generateNormals(){

	if (m_hasNormals) return;

	Vector3f normal;

	Vector3f position0;
	Vector3f position1;
	Vector3f position2;

	Vector3f normal0;
	Vector3f normal1;
	Vector3f normal2;

	Vector3f edge1;
	Vector3f edge2;

	const unsigned int *pTriangle = 0;

	std::vector<Vector3f> normals = std::vector<Vector3f>(m_indexBuffer.size());

	for (int i = 0; i < m_indexBuffer.size() / 3; i++){


		pTriangle = &m_indexBuffer[i * 3];

		position0 = m_positions[pTriangle[0]];
		position1 = m_positions[pTriangle[1]];
		position2 = m_positions[pTriangle[2]];

		edge1 = position1 - position0;
		edge2 = position2 - position0;

		normal = Vector3f::cross(edge2, edge1);

		normals[pTriangle[0]] = normals[pTriangle[0]] + normal;
		normals[pTriangle[1]] = normals[pTriangle[1]] + normal;
		normals[pTriangle[2]] = normals[pTriangle[2]] + normal;

	}

	

	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

			pTriangle = &m_indexBuffer[i * 3];

			Vector3f::normalize(normals[pTriangle[0]]);
			Vector3f::normalize(normals[pTriangle[1]]);
			Vector3f::normalize(normals[pTriangle[2]]);

			m_triangles[i]->setNormal(normals[pTriangle[0]], normals[pTriangle[1]], normals[pTriangle[2]]);


		}
	

	m_hasNormals = true;
}

void MeshSpiral::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	bounds = true;
}

void MeshSpiral::hit(Hit &hit){

	

	// find the nearest intersection
	m_KDTree->intersectRec(hit);
	
}

void MeshSpiral::setColor(Color color){
	m_color = color;
	m_defaultColor = false;
}

void MeshSpiral::repeatTexture(bool repeatTexture){

	m_repeatTexture = repeatTexture;
}

Color MeshSpiral::getColor(const Vector3f& a_pos){

	

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

std::pair<float, float> MeshSpiral::getUV(const Vector3f& a_pos){

	return m_KDTree->m_primitive->getUV(a_pos);
}

Vector3f MeshSpiral::getNormal(const Vector3f& a_pos){

	if (m_hasNormals){

		return (m_KDTree->m_primitive->getNormal(a_pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSpiral::getTangent(const Vector3f& a_pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getTangent(a_pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSpiral::getBiTangent(const Vector3f& a_pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getBiTangent(a_pos)).normalize();
	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}


Vector3f MeshSpiral::getNormalDu(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDu(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSpiral::getNormalDv(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDv(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}