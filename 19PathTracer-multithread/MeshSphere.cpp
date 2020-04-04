#include "MeshSPhere.h"
#include "KDTree.h"

MeshSphere::MeshSphere(float radius, bool generateTexels, bool generateNormals, bool generateTangents, bool  generateNormalDerivatives) : Primitive(){

	m_radius = radius;
	m_invRadius = 1.0 / radius;
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
	m_uResolution = 49;
	m_vResolution = 49;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;

}

MeshSphere::MeshSphere(float radius) : MeshSphere(radius, true, true, true, false){ }

MeshSphere::~MeshSphere(){}

void MeshSphere::setPrecision(int uResolution, int vResolution){

	m_uResolution = uResolution;
	m_vResolution = vResolution;
}

void MeshSphere::buildMesh(){

	if (m_isInitialized) return;

	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;
	std::vector<Vector3f> normalsDu;
	std::vector<Vector3f> normalsDv;

	float uAngleStep = (2.0f * PI) / float(m_uResolution);
	float vAngleStep = PI / float(m_vResolution);

	float vSegmentAngle;
	for (unsigned int i = 0; i <= m_vResolution; i++){
	
		vSegmentAngle =  i * vAngleStep;
		float cosVSegment = cosf(vSegmentAngle);
		float sinVSegment = sinf(vSegmentAngle);
		
		for (int j = 0; j <= m_uResolution; j++){

			float uSegmentAngle = j * uAngleStep;

			float cosUSegment = cosf(uSegmentAngle);
			float sinUSegment = sinf(uSegmentAngle);
			
			// Calculate vertex position on the surface of torus
			float x = m_radius * sinVSegment * cosUSegment;
			float z = m_radius * sinVSegment * sinUSegment;
			float y = m_radius * cosVSegment;

			xmin = min(x, xmin); ymin = min(y, ymin); zmin = min(z, zmin);
			xmax = max(x, xmax); ymax = max(y, ymax); zmax = max(z, zmax);

			Vector3f surfacePosition = Vector3f(x, y, z);
			m_positions.push_back(surfacePosition);
		}
	}

	if (m_generateNormals){
	
		for (unsigned int i = 0; i <= m_vResolution; i++){
		
			// starting from pi/2 to -pi/2
			vSegmentAngle = i * vAngleStep;

			float cosVSegment = cosf(vSegmentAngle);
			float sinVSegment = sinf(vSegmentAngle);

			for (int j = 0; j <= m_uResolution; j++){

				float uSegmentAngle = j * uAngleStep;

				float cosUSegment = cosf(uSegmentAngle);
				float sinUSegment = sinf(uSegmentAngle);

				// Calculate vertex position on the surface of the sphere
				float x = sinVSegment * cosUSegment;
				float y = cosVSegment;
				float z = sinVSegment * sinUSegment;
				

				Vector3f normal = Vector3f(x, y, z);
				m_normals.push_back(normal);

				/*float n1 = cosVSegment * cosVSegment * cosUSegment;
				float n2 = sinVSegment;
				float n3 = cosVSegment * cosVSegment * sinUSegment;

				Vector3f normal = Vector3f(n1 , n2, n3).normalize();
				m_normals.push_back(normal);*/
			}
		
		
		}
		m_hasNormals = true;
	}

	if (m_generateTexels){
	
		for (unsigned int i = 0; i <= m_vResolution; i++){

			for (int j = 0; j <= m_uResolution; j++){

				// Calculate vertex position on the surface of torus
				float u = (float)j / m_uResolution;
				float v = (float)i / m_vResolution;
			
				Vector2f textureCoordinate = Vector2f(u, v);
				m_texels.push_back(textureCoordinate);
			}
		}
		m_hasTexels = true;
	}


	if (m_generateTangents){
	
		for (unsigned int i = 0; i <= m_vResolution; i++){

			vSegmentAngle =  i * vAngleStep;

			float cosVSegment = cosf(vSegmentAngle);
			float sinVSegment = sinf(vSegmentAngle);

			for (int j = 0; j <= m_uResolution; j++){

				float uSegmentAngle = j * uAngleStep;

				float cosUSegment = cosf(uSegmentAngle);
				float sinUSegment = sinf(uSegmentAngle);

				tangents.push_back(Vector3f(-sinUSegment * sinVSegment, 0.0, cosUSegment* sinVSegment).normalize());
				bitangents.push_back(Vector3f(cosVSegment*cosUSegment, -sinVSegment, cosVSegment*sinUSegment).normalize());
			}
		}

		m_hasTangents = true;
	}

	if (m_generateNormalDerivatives){

		for (unsigned int i = 0; i <= m_vResolution; i++){

			vSegmentAngle =  i * vAngleStep;

			float cosVSegment = cosf(vSegmentAngle);
			float sinVSegment = sinf(vSegmentAngle);

			for (int j = 0; j <= m_uResolution; j++){

				float uSegmentAngle = j * uAngleStep;

				float cosUSegment = cosf(uSegmentAngle);
				float sinUSegment = sinf(uSegmentAngle);

				
				/*Vector3f tangent = Vector3f(-sinUSegment * sinVSegment, 0.0, cosUSegment* sinVSegment).normalize();
				Vector3f bitangent = Vector3f(cosVSegment*cosUSegment, -sinVSegment, cosVSegment*sinUSegment).normalize();
				Vector3f normal = Vector3f::cross(tangent, bitangent);

				Vector3f d2Pduu = -Vector3f(sinVSegment * cosUSegment, 0.0, sinVSegment * sinUSegment);
				Vector3f d2Pdvv = -Vector3f(sinVSegment * cosUSegment, cosVSegment, sinVSegment * sinUSegment);
				Vector3f d2Pduv = Vector3f(-cosVSegment * sinUSegment, 0.0, cosVSegment * cosUSegment);

				float E = Vector3f::dot(tangent, tangent);
				float F = Vector3f::dot(tangent, bitangent);
				float G = Vector3f::dot(bitangent, bitangent);

				float e = Vector3f::dot(normal, d2Pduu);
				float f = Vector3f::dot(normal, d2Pduv);
				float g = Vector3f::dot(normal, d2Pdvv);

				float invEGF2 = 1.0 / (E * G - F * F);
				Vector3f dndu = ((f * F - e * G) * invEGF2 * tangent + (e * F - f * E) * invEGF2 * bitangent).normalize();
				Vector3f dndv = ((g * F - f * G) * invEGF2 * tangent + (f * F - g * E) * invEGF2 * bitangent).normalize();

				normalsDu.push_back(dndu);
				normalsDv.push_back(dndv);*/

				float n1u = -sinVSegment * sinUSegment;
				float n3u =  sinVSegment * cosUSegment;

				Vector3f dndu = Vector3f(n1u, 0.0, n3u).normalize();
				normalsDu.push_back(dndu);

			
				float n1v = cosVSegment  * cosUSegment;
				float n2v = -sinVSegment;
				float n3v = cosVSegment  * sinUSegment;

				Vector3f dndv = Vector3f( n1v, n2v, n3v).normalize();
				normalsDv.push_back(dndv);

			}
		}

		m_hasNormalDerivatives = true;
	}

	//calculate the indices
	for (unsigned int j = 0; j < m_uResolution; j++){

		if (j == (m_uResolution - 1)){

			m_indexBuffer.push_back(0);
			m_indexBuffer.push_back((m_uResolution + 1) + j);
			m_indexBuffer.push_back((m_uResolution + 1));

		}else{
			m_indexBuffer.push_back(0);
			m_indexBuffer.push_back((m_uResolution + 1) + j);
			m_indexBuffer.push_back((m_uResolution + 1) + j + 1);
		}
	}


	for (unsigned int i = 1; i < m_vResolution - 1; i++){

		int k1 = i * (m_uResolution + 1);   
		int k2 = k1 + (m_uResolution + 1) ;   

		for (unsigned int j = 0; j < m_uResolution; j++){
		
			if (j == (m_uResolution - 1)){
				
				m_indexBuffer.push_back(k1 + j);
				m_indexBuffer.push_back(k2 + j);
				m_indexBuffer.push_back(k1);

				m_indexBuffer.push_back(k1);
				m_indexBuffer.push_back(k2 + j);
				m_indexBuffer.push_back(k2 );
		
			}else{

				m_indexBuffer.push_back(k1 + j);
				m_indexBuffer.push_back(k2 + j);
				m_indexBuffer.push_back(k1 + j + 1);

				m_indexBuffer.push_back(k1 + j + 1);
				m_indexBuffer.push_back(k2 + j);
				m_indexBuffer.push_back(k2 + j + 1);
			}
		}
	}

	for (unsigned int j = 0; j < m_uResolution; j++){

		if (j == (m_uResolution - 1)){

			m_indexBuffer.push_back((m_vResolution - 1) * (m_uResolution + 1) + j);
			m_indexBuffer.push_back((m_vResolution - 1) * (m_uResolution + 1));
			m_indexBuffer.push_back(m_vResolution * (m_uResolution + 1));

		}else{

			m_indexBuffer.push_back((m_vResolution - 1) * (m_uResolution + 1) + j);
			m_indexBuffer.push_back((m_vResolution - 1) * (m_uResolution + 1) + j + 1);
			m_indexBuffer.push_back(m_vResolution * (m_uResolution + 1));
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

		triangle->setColor(m_color);
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

void MeshSphere::generateNormals(){

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

	m_normals = std::vector<Vector3f>(m_indexBuffer.size());

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

void MeshSphere::generateTangents(){

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

	tangents = std::vector<Vector3f>(m_indexBuffer.size());
	bitangents = std::vector<Vector3f>(m_indexBuffer.size());

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

			tangent = ((edge02 * texEdge12[1]) - (edge12 * texEdge02[1])) * det;
			bitangent = (-(edge02 * texEdge12[0]) + (edge12 * texEdge02[0])) * det;

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
		/*nDotT = m_normals[i][0] * tangents[i][0] +
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
		}*/

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

void MeshSphere::generateNormalDerivatives(){


	if (m_hasNormalDerivatives){ std::cout << "Normal-Derivatives already generated!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
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

	std::vector<Vector3f> normalsDu;
	std::vector<Vector3f> normalsDv;

	normalsDu = std::vector<Vector3f>(m_indexBuffer.size());
	normalsDv = std::vector<Vector3f>(m_indexBuffer.size());

	for (int i = 0; i < m_indexBuffer.size() / 3; i++){

		pTriangle = &m_indexBuffer[i * 3];

		normal0 = m_normals[pTriangle[0]];
		normal1 = m_normals[pTriangle[1]];
		normal2 = m_normals[pTriangle[2]];


		// Calculate the triangle face normalDu and normalDv.
		edge02 = normal0 - normal2;
		edge12 = normal1 - normal2;

		texEdge02 = m_texels[pTriangle[0]] - m_texels[pTriangle[2]];
		texEdge12 = m_texels[pTriangle[1]] - m_texels[pTriangle[2]];

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

			normalDu = ((edge02 * texEdge12[1]) - (edge12 * texEdge02[1])) * det;
			normalDv = ((-edge02 * texEdge12[0]) + (edge12 * texEdge02[0])) * det;
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
}

void MeshSphere::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	bounds = true;
}

void MeshSphere::hit(Hit &hit){
	// find the nearest intersection
	m_KDTree->intersectRec(hit);

}

void MeshSphere::setColor(Color color){
	m_color = color;
	m_defaultColor = false;
}


Color MeshSphere::getColor(const Vector3f& pos){

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

std::pair<float, float> MeshSphere::getUV(const Vector3f& pos){

	return m_KDTree->m_primitive->getUV(pos);
}

Vector3f MeshSphere::getNormal(const Vector3f& pos){

	if (m_hasNormals){

		return (m_KDTree->m_primitive->getNormal(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSphere::getTangent(const Vector3f& pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getTangent(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSphere::getBiTangent(const Vector3f& pos){

	if (m_hasTangents){

		return (m_KDTree->m_primitive->getBiTangent(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSphere::getNormalDu(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDu(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f MeshSphere::getNormalDv(const Vector3f& pos){

	if (m_hasNormalDerivatives){

		return (m_KDTree->m_primitive->getNormalDv(pos)).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}