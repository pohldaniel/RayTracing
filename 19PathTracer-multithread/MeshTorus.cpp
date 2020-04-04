#include "MeshTorus.h"
#include "KDTree.h"

MeshTorus::MeshTorus(float radius, float tubeRadius, bool generateTexels, bool generateNormals, bool generateTangents, bool  generateNormalDerivatives) : Primitive(){

	m_radius = radius;
	m_tubeRadius = tubeRadius;
	m_generateNormals = generateNormals;
	m_generateTexels = generateTexels;
	m_generateTangents = generateTangents;
	m_generateNormalDerivatives = generateNormalDerivatives;

	m_hasTexels = false;
	m_hasNormals = false;
	m_hasTangents = false;
	m_hasNormalDerivatives = false;

	m_defaultColor = true;
	m_isInitialized = false;
	m_mainSegments = 25;
	m_tubeSegments = 25;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;

}

MeshTorus::MeshTorus(float radius, float tubeRadius) : MeshTorus(radius, tubeRadius, true, true, true, false){ }

MeshTorus::~MeshTorus(){}

void MeshTorus::setPrecision(int mainSegments, int tubeSegments){

	m_mainSegments = mainSegments;
	m_tubeSegments = tubeSegments;
}

//http://www.mbsoftworks.sk/tutorials/opengl4/011-indexed-rendering-torus/
//https://de.wikipedia.org/wiki/Torus
void MeshTorus::buildMesh(){

	if (m_isInitialized) return;

	
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;
	std::vector<Vector3f> normalsDu;
	std::vector<Vector3f> normalsDv;

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

			// Calculate vertex position on the surface of torus
			float x = (m_radius + m_tubeRadius * cosTubeSegment)*cosMainSegment;
			float z = (m_radius + m_tubeRadius * cosTubeSegment)*sinMainSegment;
			float y = m_tubeRadius*sinTubeSegment;

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

	if (m_generateTexels){
	
		float mainSegmentTextureStep = 1.0f / (float(m_mainSegments));
		float tubeSegmentTextureStep = 1.0f / (float(m_tubeSegments));

		//rotate the texture to get the same mapping like the primitive
		float currentMainSegmentTexCoordV = 0.0 - 0.25;

		for (unsigned int i = 0; i <= m_mainSegments; i++){

			//rotate the texture to get the same mapping like the primitive
			float currentTubeSegmentTexCoordU = 0.0 - 0.5f;

			for (unsigned int j = 0; j <= m_tubeSegments; j++){
				
				Vector2f textureCoordinate = Vector2f(1.0 - currentMainSegmentTexCoordV, currentTubeSegmentTexCoordU);
				m_texels.push_back(textureCoordinate);
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

				float tmp = m_radius + m_tubeRadius *cosTubeSegment;

				Vector3f normal = tmp * Vector3f(cosMainSegment*cosTubeSegment, sinTubeSegment, sinMainSegment*cosTubeSegment);
				m_normals.push_back(normal);

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


	if (m_generateNormalDerivatives){
		
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

				//first way: Weingarten equations see pbrt section 3.2.3

				/*Vector3f normal = Vector3f(cosMainSegment*cosTubeSegment, sinTubeSegment, sinMainSegment*cosTubeSegment);
				Vector3f tangent = Vector3f(-sinTubeSegment *cosMainSegment, cosTubeSegment, -sinTubeSegment *sinMainSegment);
				Vector3f bitangent = Vector3f(-sinMainSegment, 0.0, cosMainSegment);

				Vector3f d2Pduu = -Vector3f(cosTubeSegment *cosMainSegment, sinTubeSegment, cosTubeSegment *sinMainSegment);
				Vector3f d2Pdvv = -Vector3f(cosMainSegment, 0.0, sinMainSegment);
				Vector3f d2Pduv = Vector3f(sinMainSegment * sinTubeSegment, 0.0, -sinTubeSegment*cosMainSegment);

				float E = Vector3f::dot(tangent, tangent);
				float F = Vector3f::dot(tangent, bitangent);
				float G = Vector3f::dot(bitangent, bitangent);

				float e = Vector3f::dot(normal, d2Pduu);
				float f = Vector3f::dot(normal, d2Pduv);
				float g = Vector3f::dot(normal, d2Pdvv);

				float invEGF2 = 1.0 / (E * G - F * F);
				int sgn = (invEGF2 >= 0) ? 1 : -1;
				Vector3f dndu = ((f * F - e * G) * sgn * tangent + (e * F - f * E) * sgn * bitangent).normalize();
				Vector3f dndv = ((g * F - f * G) * sgn * tangent + (f * F - g * E) * sgn * bitangent).normalize();

				normalsDu.push_back(dndu);
				normalsDv.push_back(dndv);*/

				//second way: dn/du		and dn/dv	
				float tmp = (m_radius + 2 * m_tubeRadius * cosTubeSegment);

				float n1u = -tmp * cosMainSegment * sinTubeSegment;
				float n2u = tmp * cosTubeSegment -  m_tubeRadius;
				float n3u = -tmp * sinMainSegment * sinTubeSegment;
				Vector3f dndu = Vector3f(n1u, n2u, n3u).normalize();
				normalsDu.push_back(dndu);

				int sgn = ((m_radius + m_tubeRadius *cosTubeSegment) *cosTubeSegment >= 0) ? 1 : -1;
				Vector3f dndv = Vector3f(-sinMainSegment * sgn, 0.0, cosMainSegment * sgn);
				normalsDv.push_back(dndv);

				// Update current tube angle
				currentTubeSegmentAngle += tubeSegmentAngleStep;
			}
			// Update main segment angle
			currentMainSegmentAngle += mainSegmentAngleStep;
		}

		m_hasNormalDerivatives = true;
	}

	
	//calculate the indices
	unsigned int currentVertexOffset = 0;
	for (unsigned int i = 0; i < m_mainSegments; i++){

		for (unsigned int j = 0; j <= m_tubeSegments; j++){

			unsigned int vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD, vertexIndexE, vertexIndexF;

			if ((j > 0) && ((j + 1) % (m_tubeSegments + 1)) == 0){

				currentVertexOffset = ((i + 1) * (m_tubeSegments + 1 ));
				
			}else{

				vertexIndexA = currentVertexOffset;
				vertexIndexB = currentVertexOffset + m_tubeSegments + 1;
				vertexIndexC = currentVertexOffset + 1;

				vertexIndexD = currentVertexOffset + m_tubeSegments + 1;
				vertexIndexF = currentVertexOffset + m_tubeSegments + 2;
				vertexIndexE = currentVertexOffset + 1;

				m_indexBuffer.push_back(vertexIndexA); m_indexBuffer.push_back(vertexIndexC); m_indexBuffer.push_back(vertexIndexB);
				m_indexBuffer.push_back(vertexIndexD); m_indexBuffer.push_back(vertexIndexF); m_indexBuffer.push_back(vertexIndexE);
				currentVertexOffset++;
			}
		}		
	}

	//set up the faces
	std::shared_ptr<Triangle> triangle;
	for (unsigned int i = 0; i < m_indexBuffer.size(); i = i + 3){

		triangle = std::shared_ptr<Triangle>(new Triangle(m_positions[m_indexBuffer[i]], m_positions[m_indexBuffer[i + 1]], m_positions[m_indexBuffer[i + 2]], false, true)); 
		
		if (m_hasNormals){
			triangle->setNormal(m_normals[m_indexBuffer[i]], m_normals[m_indexBuffer[i + 1]], m_normals[m_indexBuffer[i + 2]]);
		}
		
		if (m_hasTexels){
			triangle->setUV(m_texels[m_indexBuffer[i]], m_texels[m_indexBuffer[i + 1]], m_texels[m_indexBuffer[i + 2]]);
		}

		if (m_hasTangents){

			triangle->setTangents(tangents[m_indexBuffer[i]], tangents[m_indexBuffer[i + 1]], tangents[m_indexBuffer[i + 2]]);
			triangle->setBiTangents(bitangents[m_indexBuffer[i]], bitangents[m_indexBuffer[i + 1]], bitangents[m_indexBuffer[i + 2]]);
		}

		if (m_generateNormalDerivatives){

			triangle->setNormalsDu(normalsDu[m_indexBuffer[i]], normalsDu[m_indexBuffer[i + 1]], normalsDu[m_indexBuffer[i + 2]]);
			triangle->setNormalsDv(normalsDv[m_indexBuffer[i]], normalsDv[m_indexBuffer[i + 1]], normalsDv[m_indexBuffer[i + 2]]);
		}

		triangle->setColor(m_color/(i+ 1));
		triangle->m_texture = m_texture;
		triangle->m_material = m_material;

		m_triangles.push_back(triangle);
	}

	tangents.clear();
	bitangents.clear();
	normalsDu.clear();
	normalsDv.clear();

	calcBounds();
	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	m_isInitialized = true;
}

void MeshTorus::generateNormals(){

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

	int maxIndex = *std::max_element(m_indexBuffer.begin(), m_indexBuffer.end());
	
	for (int i = 0; i < maxIndex; i++){

		m_normals.push_back(Vector3f(0.0, 0.0, 0.0));		
	}
	
	for (int i = 0; i < m_indexBuffer.size() / 3; i++){


		pTriangle = &m_indexBuffer[i * 3];

		position0 = m_positions[pTriangle[0]];
		position1 = m_positions[pTriangle[1]];
		position2 = m_positions[pTriangle[2]];

		edge1 = position1 - position0;
		edge2 = position2 - position0;

		normal = Vector3f::cross(edge2, edge1);

		m_normals[pTriangle[0]] = m_normals[pTriangle[0]] + normal;
		m_normals[pTriangle[1]] = m_normals[pTriangle[1]] + normal;
		m_normals[pTriangle[2]] = m_normals[pTriangle[2]] + normal;

	}



	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		pTriangle = &m_indexBuffer[i * 3];

		Vector3f::normalize(m_normals[pTriangle[0]]);
		Vector3f::normalize(m_normals[pTriangle[1]]);
		Vector3f::normalize(m_normals[pTriangle[2]]);

		m_triangles[i]->setNormal(m_normals[pTriangle[0]], m_normals[pTriangle[1]], m_normals[pTriangle[2]]);


	}

	m_hasNormals = true;
}

void MeshTorus::generateTangents(){

	if (m_hasTangents){ std::cout << "Tangents already generated!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;

	}

	Vector3f position0;
	Vector3f position1;
	Vector3f position2;

	Vector3f edge02;
	Vector3f edge12;

	Vector2f texEdge02;
	Vector2f texEdge12;

	Vector3f normal;
	Vector3f  tangent;
	Vector3f  bitangent;

	float det = 0.0f;
	float nDotT = 0.0f;
	float length = 0.0f;
	float bDotB = 0.0f;

	const unsigned int *pTriangle = 0;
	
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;

	int maxIndex = *std::max_element(m_indexBuffer.begin(), m_indexBuffer.end());

	for (int i = 0; i < maxIndex; i++){

		tangents.push_back(Vector3f(0.0, 0.0, 0.0));
		bitangents.push_back(Vector3f(0.0, 0.0, 0.0));
	}

	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		pTriangle = &m_indexBuffer[i * 3];
	
		position0 = m_positions[pTriangle[0]];
		position1 = m_positions[pTriangle[1]];
		position2 = m_positions[pTriangle[2]];

		// Calculate the triangle face tangent and bitangent.
		edge02 = position0 - position2;
		edge12 = position1 - position2;

		texEdge02 = m_texels[pTriangle[0]] - m_texels[pTriangle[2]];
		texEdge12 = m_texels[pTriangle[1]] - m_texels[pTriangle[2]];

		det = texEdge02[0] * texEdge12[1] - texEdge02[1] * texEdge12[0];

		if (fabs(det) < 1e-6f){

			tangent[0] = 1.0f;
			tangent[1] = 0.0f;
			tangent[2] = 0.0f;

			bitangent[0] = 0.0f;
			bitangent[1] = 1.0f;
			bitangent[2] = 0.0f;

		}else{

			det = 1.0f / det;

			tangent = ((edge12 * texEdge02[0]) - (edge02 * texEdge12[0])) * det;
			bitangent = (-(edge02 * texEdge12[1]) + (edge12 * texEdge02[1])) * det;

		}

		// Accumulate the tangents and bitangents.
		tangents[pTriangle[0]] = tangents[pTriangle[0]] + tangent;
		tangents[pTriangle[1]] = tangents[pTriangle[1]] + tangent;
		tangents[pTriangle[2]] = tangents[pTriangle[2]] + tangent;

		bitangents[pTriangle[0]] = bitangents[pTriangle[0]] + bitangent;
		bitangents[pTriangle[1]] = bitangents[pTriangle[1]] + bitangent;
		bitangents[pTriangle[2]] = bitangents[pTriangle[2]] + bitangent;
	}

	// Orthogonalize and normalize the vertex tangents.
	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		// Gram-Schmidt orthogonalize tangent with normal.
		nDotT = m_normals[i][0] * tangents[i][0] +
			m_normals[i][1] * tangents[i][1] +
			m_normals[i][2] * tangents[i][2];

		tangents[i][0] -= m_normals[i][0] * nDotT;
		tangents[i][1] -= m_normals[i][1] * nDotT;
		tangents[i][2] -= m_normals[i][2] * nDotT;

		// Normalize the tangent.
		Vector3f::normalize(tangents[i]);
		bitangent[0] = (m_normals[i][1] * tangents[i][2]) -
			(m_normals[i][2] * tangents[i][1]);
		bitangent[1] = (m_normals[i][2] * tangents[i][0]) -
			(m_normals[i][0] * tangents[i][2]);
		bitangent[2] = (m_normals[i][0] * tangents[i][1]) -
			(m_normals[i][1] * tangents[i][0]);

		bDotB = bitangent[0] * bitangents[i][0] +
			bitangent[1] * bitangents[i][1] +
			bitangent[2] * bitangents[i][2];

		// Calculate handedness
		tangents[i][3] = (bDotB < 0.0f) ? 1.0f : -1.0f;
		bitangents[i] = bitangent;

		if (bDotB < 0.0f){
			tangents[i] = -tangents[i];
		}

		pTriangle = &m_indexBuffer[i * 3];

		Vector3f::normalize(tangents[pTriangle[0]]);
		Vector3f::normalize(tangents[pTriangle[1]]);
		Vector3f::normalize(tangents[pTriangle[2]]);

		Vector3f::normalize(bitangents[pTriangle[0]]);
		Vector3f::normalize(bitangents[pTriangle[1]]);
		Vector3f::normalize(bitangents[pTriangle[2]]);
	}

	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		pTriangle = &m_indexBuffer[i * 3];

		m_triangles[i]->setTangents(tangents[pTriangle[0]], tangents[pTriangle[1]], tangents[pTriangle[2]]);
		m_triangles[i]->setBiTangents(bitangents[pTriangle[0]], bitangents[pTriangle[1]], bitangents[pTriangle[2]]);
	}


	m_hasNormals = true;
	m_hasTangents = true;

	tangents.clear();
	bitangents.clear();

}

void MeshTorus::generateNormalDerivatives(){
	

	if (m_hasNormalDerivatives){ std::cout << "Normal-Derivatives already generated!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;
	}

	Vector3f normal0;
	Vector3f normal1;
	Vector3f normal2;

	Vector3f edge02;
	Vector3f edge12;

	Vector2f texEdge02;
	Vector2f texEdge12;

	Vector3f normal;
	Vector3f normalDu;
	Vector3f normalDv;

	const unsigned int *pTriangle = 0;

	float det = 0.0f;
	float mainSegmentTextureStep = 1.0f / (float(m_mainSegments));
	float tubeSegmentTextureStep = 1.0f / (float(m_tubeSegments));

	std::vector<Vector3f> normalsDu;
	std::vector<Vector3f> normalsDv;
	std::vector<Vector2f> texels;

	int maxIndex = *std::max_element(m_indexBuffer.begin(), m_indexBuffer.end());

	for (int i = 0; i < maxIndex; i++){

		normalsDu.push_back(Vector3f(0.0, 0.0, 0.0));
		normalsDv.push_back(Vector3f(0.0, 0.0, 0.0));
	}

	//calculate texture coords
	float currentMainSegmentTexCoordV = 0.0;
	for (unsigned int i = 0; i <= m_mainSegments; i++){

		
		float currentTubeSegmentTexCoordU = 0.0;
		for (unsigned int j = 0; j <= m_tubeSegments; j++){

			Vector2f textureCoordinate = Vector2f(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV);
			texels.push_back(textureCoordinate);
			currentTubeSegmentTexCoordU += tubeSegmentTextureStep;
		}

		// Update texture coordinate of main segment
		currentMainSegmentTexCoordV += mainSegmentTextureStep;
	}


	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		pTriangle = &m_indexBuffer[i * 3];
		
		normal0 = m_normals[pTriangle[0]];
		normal1 = m_normals[pTriangle[1]];
		normal2 = m_normals[pTriangle[2]];

		
		// Calculate the triangle face normalDu and normalDv.

		//edge02 = normal0 - normal2;
		//edge12 = normal1 - normal2;	

		edge02 = normal1 - normal0;
		edge12 = normal2 - normal0;

		//texEdge02 = texels[pTriangle[0]] - texels[pTriangle[2]];
		//texEdge12 = texels[pTriangle[1]] - texels[pTriangle[2]];

		texEdge02 = texels[pTriangle[1]] - texels[pTriangle[0]];
		texEdge12 = texels[pTriangle[2]] - texels[pTriangle[0]];

		det = texEdge02[0] * texEdge12[1] - texEdge02[1] * texEdge12[0];

		if (fabs(det) < 1e-6f){
			
			normalDu[0] = 1.0f;
			normalDu[1] = 0.0f;
			normalDu[2] = 0.0f;

			normalDv[0] = 0.0f;
			normalDv[1] = 1.0f;
			normalDv[2] = 0.0f;

		}else{
			det = 1.0f / det;

			normalDu = (edge02 * texEdge12[1] - edge12 * texEdge02[1]) * det;
			normalDv = ((edge12 * texEdge02[0]) - (edge02 * texEdge12[0])) * det;

			//normalDu = ((edge02 * texEdge12[1]) - (edge12 * texEdge02[1])) * det;
			//normalDv = ((-edge02 * texEdge12[0]) + (edge12 * texEdge02[0])) * det;	
		}
		

		// Accumulate the normalsDu and normalsDv
		normalsDu[pTriangle[0]] = normalsDu[pTriangle[0]] + normalDu;
		normalsDu[pTriangle[1]] = normalsDu[pTriangle[1]] + normalDu;
		normalsDu[pTriangle[2]] = normalsDu[pTriangle[2]] + normalDu;

		normalsDv[pTriangle[0]] = normalsDv[pTriangle[0]] + normalDv;
		normalsDv[pTriangle[1]] = normalsDv[pTriangle[1]] + normalDv;
		normalsDv[pTriangle[2]] = normalsDv[pTriangle[2]] + normalDv;
	}

	
	//Normalize the Normal-Derivatives.
	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

			pTriangle = &m_indexBuffer[i * 3];

			Vector3f::normalize(normalsDu[pTriangle[0]]); Vector3f::normalize(normalsDu[pTriangle[1]]); Vector3f::normalize(normalsDu[pTriangle[2]]);
			Vector3f::normalize(normalsDv[pTriangle[0]]); Vector3f::normalize(normalsDv[pTriangle[1]]); Vector3f::normalize(normalsDv[pTriangle[2]]);

			m_triangles[i]->setNormalsDu(normalsDu[pTriangle[0]], normalsDu[pTriangle[1]], normalsDu[pTriangle[2]]);
			m_triangles[i]->setNormalsDv(normalsDv[pTriangle[0]], normalsDv[pTriangle[1]], normalsDv[pTriangle[2]]);
	}
	
	m_hasNormals = true;
	m_hasNormalDerivatives = true;

	normalsDu.clear();
	normalsDv.clear();
	texels.clear();
	
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


Color MeshTorus::getColor(const Vector3f& pos){

	// use one texture for the whole model
	if (m_texture){

		m_KDTree->m_primitive->m_texture = m_texture;
		return m_KDTree->m_primitive->getColor(pos);

	// use one texture per mesh
	// maybe the texture isn't at the path of the mlt file, then a nulltexure will created
	}else if (m_KDTree->m_primitive->m_texture && m_useTexture){

		return m_KDTree->m_primitive->getColor(pos);

	// use one color for the whole model
	}else if (!m_defaultColor) {
		
		return m_color;

	// use the color of every single triangle
	// the value is different for every mesh
	}else{

		return m_KDTree->m_primitive->m_color;
	}

}

std::pair<float, float> MeshTorus::getUV(const Vector3f& pos){

	return m_KDTree->m_primitive->getUV(pos);
}

Vector3f MeshTorus::getNormal(const Vector3f& pos){

	if (m_hasNormals){

		return (m_KDTree->m_primitive->getNormal(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshTorus::getTangent(const Vector3f& pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getTangent(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshTorus::getBiTangent(const Vector3f& pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getBiTangent(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshTorus::getNormalDu(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDu(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshTorus::getNormalDv(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDv(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}