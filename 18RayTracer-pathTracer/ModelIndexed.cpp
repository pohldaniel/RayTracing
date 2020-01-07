#include "ModelIndexed.h"
#include "KDTree.h"

ModelIndexed::ModelIndexed() : Primitive() {

	m_hasMaterials = false;
	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;
	m_hasNormalDerivatives = false;
	m_defaultColor = true;

	m_texture = NULL;
	m_material = NULL;

	xmin = FLT_MAX; ymin = FLT_MAX; zmin = FLT_MAX;
	xmax = -FLT_MAX; ymax = -FLT_MAX; zmax = -FLT_MAX;
}


ModelIndexed::~ModelIndexed(){
}

void ModelIndexed::setColor(Color color){
	m_color = color;
	m_defaultColor = false;
}

void ModelIndexed::hit(Hit &hit){
	// find the nearest intersection
	m_KDTree->intersectRec(hit);

}

void ModelIndexed::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	ModelIndexed::bounds = true;
}

std::pair <float, float> ModelIndexed::getUV(const Vector3f& pos){

	return m_KDTree->m_primitive->getUV(pos);
}

Color ModelIndexed::getColor(const Vector3f& pos){

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

Vector3f ModelIndexed::getNormal(const Vector3f& pos){

	if (m_hasNormals){
		return (m_KDTree->m_primitive->getNormal(pos)).normalize();

	}else{
		return Vector3f(0.0, 0.0, 0.0);
	}

}

Vector3f ModelIndexed::getTangent(const Vector3f& pos){

	if (m_hasTangents){
		return (m_KDTree->m_primitive->getTangent(pos)).normalize();

	}else{
		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f ModelIndexed::getBiTangent(const Vector3f& pos){

	if (m_hasTangents){
		return (m_KDTree->m_primitive->getBiTangent(pos)).normalize();
	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f ModelIndexed::getNormalDu(const Vector3f& pos){

	if (m_hasNormalDerivatives){
		return (m_KDTree->m_primitive->getNormalDu(pos)).normalize();

	}else{
		return Vector3f(0.0, 0.0, 0.0);
	}
}
Vector3f ModelIndexed::getNormalDv(const Vector3f& pos){

	if (m_hasNormalDerivatives){
		return (m_KDTree->m_primitive->getNormalDv(pos)).normalize();

	}else{
		return Vector3f(0.0, 0.0, 0.0);
	}
}

std::shared_ptr<Material>  ModelIndexed::getMaterialMesh(){

	if (m_materialMesh){
		return m_materialMesh;
	}

	return std::shared_ptr<Material>(new Phong());
}

std::shared_ptr<Material> ModelIndexed::getMaterial(){

	if (m_material){
		return m_material;

	}else{
		return m_KDTree->m_primitive->m_material;
	}
}

bool compare2(const std::array<int, 10> &i_lhs, const std::array<int, 10> &i_rhs){

	return i_lhs[9] < i_rhs[9];
}

bool ModelIndexed::loadObject(const char* filename, bool cull, bool smooth){

	return loadObject(filename, Vector3f(0.0, 0.0, 1.0), 0.0, Vector3f(0.0, 0.0, 0.0), 1.0, cull, smooth);
}



bool ModelIndexed::loadObject(const char* a_filename, Vector3f &axis, float degree, Vector3f &translate, float scale, bool cull, bool smooth){

	std::string filename(a_filename);

	const size_t index = filename.rfind('/');

	if (std::string::npos != index){

		m_modelDirectory = filename.substr(0, index);
	}

	std::vector<std::array<int, 10>> face;

	std::vector<float> vertexCoords;
	std::vector<float> normalCoords;
	std::vector<float> textureCoords;

	std::map<std::string, int> name;

	int countMesh = 0;
	int assign = 0;
	int countFacesWithTexture = 0;

	float xmin = FLT_MAX;
	float ymin = FLT_MAX;
	float zmin = FLT_MAX;
	float xmax = -FLT_MAX;
	float ymax = -FLT_MAX;
	float zmax = -FLT_MAX;

	char buffer[250];

	FILE * pFile = fopen(a_filename, "r");
	if (pFile == NULL){
		std::cout << "File not found" << std::endl;
		return false;
	}

	while (fscanf(pFile, "%s", buffer) != EOF){

		switch (buffer[0]){

		case '#':{

					 fgets(buffer, sizeof(buffer), pFile);
					 break;

		}case 'm':{

					 fgets(buffer, sizeof(buffer), pFile);
					 sscanf(buffer, "%s %s", buffer, buffer);
					 m_mltPath = buffer;

					 m_hasMaterials = true;
					 break;

		}case 'v':{

			switch (buffer[1]){

			case '\0':{

						  float tmpx, tmpy, tmpz;
						  fgets(buffer, sizeof(buffer), pFile);
						  sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

						  tmpx = tmpx * scale + translate[0];
						  tmpy = tmpy * scale + translate[1];
						  tmpz = tmpz * scale + translate[2];

						  vertexCoords.push_back(tmpx);
						  vertexCoords.push_back(tmpy);
						  vertexCoords.push_back(tmpz);

						  xmin = (std::min)(tmpx, xmin);
						  ymin = (std::min)(tmpy, ymin);
						  zmin = (std::min)(tmpz, zmin);

						  xmax = (std::max)(tmpx, xmax);
						  ymax = (std::max)(tmpy, ymax);
						  zmax = (std::max)(tmpz, zmax);
						  break;

			}case 't':{

						  float tmpu, tmpv;
						  fgets(buffer, sizeof(buffer), pFile);
						  sscanf(buffer, "%f %f", &tmpu, &tmpv);

						  textureCoords.push_back(tmpu);
						  textureCoords.push_back(tmpv);
						  break;

			}case 'n':{

				float tmpx, tmpy, tmpz;
				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

				normalCoords.push_back(tmpx);
				normalCoords.push_back(tmpy);
				normalCoords.push_back(tmpz);
				break;

			}default:{

				break;
			}
			}
			break;

		}case 'u': {

			if (m_hasMaterials){

				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%s %s", buffer, buffer);

				std::map<std::string, int >::const_iterator iter = name.find(buffer);

				if (iter == name.end()){
					// mlt name not found
					countMesh++;
					assign = countMesh;

					name[buffer] = countMesh;

				}else{
					// mlt name found
					assign = iter->second;
				}
			}
			break;

		}case 'g': {

			if (!m_hasMaterials){

				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%s", buffer);

				countMesh++;
				assign = countMesh;
				name[buffer] = countMesh;
			}
			break;

		}case 'f': {

			int a, b, c, n1, n2, n3, t1, t2, t3;
			fgets(buffer, sizeof(buffer), pFile);

			if (!textureCoords.empty() && !normalCoords.empty()){
				sscanf(buffer, "%d/%d/%d %d/%d/%d %d/%d/%d ", &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);
				face.push_back({ { a, b, c, t1, t2, t3, n1, n2, n3, assign } });
				
			}else if (!normalCoords.empty()){
				sscanf(buffer, "%d//%d %d//%d %d//%d", &a, &n1, &b, &n2, &c, &n3);
				face.push_back({ { a, b, c, 0, 0, 0, n1, n2, n3, assign } });

			}else if (!textureCoords.empty()){
				sscanf(buffer, "%d/%d %d/%d %d/%d", &a, &t1, &b, &t2, &c, &t3);
				face.push_back({ { a, b, c, t1, t2, t3, 0, 0, 0, assign } });

			}else {
				sscanf(buffer, "%d %d %d", &a, &b, &c);
				face.push_back({ { a, b, c, 0, 0, 0, 0, 0, 0, assign } });
			}
			break;

		}default: {

			break;
		}

		}//end switch
	}// end while
	fclose(pFile);

	std::sort(face.begin(), face.end(), compare2);

	std::map<int, int> dup;

	for (unsigned int i = 0; i < face.size(); i++){
		dup[face[i][9]]++;
	}

	m_numberOfMeshes = dup.size();
	m_numberOfVertices = vertexCoords.size();
	m_numberOfTriangles = face.size();

	std::map<int, int>::const_iterator iterDup = dup.begin();

	for (iterDup; iterDup != dup.end(); iterDup++){
		if (name.empty()){
			meshes.push_back(std::shared_ptr<MeshIndexed>(new MeshIndexed(iterDup->second, this)));
		}else{

			std::map<std::string, int >::const_iterator iterName = name.begin();
			for (iterName; iterName != name.end(); iterName++){

				if (iterDup->first == iterName->second){
					meshes.push_back(std::shared_ptr<MeshIndexed>(new MeshIndexed("newmtl " + iterName->first, iterDup->second, this)));
				}
			}

		}
	}

	dup.clear();
	name.clear();

	for (int j = 0; j < m_numberOfMeshes; j++){

		if (m_material){

			meshes[j]->m_material = std::shared_ptr<Material>(m_material);

		}else if (m_hasMaterials){

			meshes[j]->m_material = std::shared_ptr<Material>(new Phong());

		}

		if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){

			if (meshes[j]->m_material->colorMapPath != ""){

				meshes[j]->m_texture = std::shared_ptr<ImageTexture>(new ImageTexture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]));

			}

			if (meshes[j]->m_material->bumpMapPath != ""){

				meshes[j]->m_material->setNormalTexture(new ImageTexture(&(m_modelDirectory + "/" + meshes[j]->m_material->bumpMapPath)[0]));
			}

		}
		meshes[j]->m_color = Color(((float)m_numberOfMeshes - 1.0) - j / (m_numberOfMeshes - 1.0), ((float)m_numberOfMeshes - 1.0) - j / (m_numberOfMeshes - 1.0), ((float)m_numberOfMeshes - 1.0) - j / (m_numberOfMeshes - 1.0));


	}// end for
	m_materialMesh = meshes[0]->m_material;

	if (!normalCoords.empty() && !textureCoords.empty()) {

		m_hasNormals = true;
		m_hasTexels = true;
		m_indexBuffer.resize(m_numberOfTriangles * 3);

		int numberOfTriangle = 0;

		for (int i = 0; i < m_numberOfTriangles; i++){
			
			float vertex1[] = { vertexCoords[((face[i])[0] - 1) * 3], vertexCoords[((face[i])[0] - 1) * 3 + 1], vertexCoords[((face[i])[0] - 1) * 3 + 2],
				textureCoords[((face[i])[3] - 1) * 2], textureCoords[((face[i])[3] - 1) * 2 + 1],
				normalCoords[((face[i])[6] - 1) * 3], normalCoords[((face[i])[6] - 1) * 3 + 1], normalCoords[((face[i])[6] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3] = addVertex(((face[i])[0] - 1), &vertex1[0], 8);

			float vertex2[] = { vertexCoords[((face[i])[1] - 1) * 3], vertexCoords[((face[i])[1] - 1) * 3 + 1], vertexCoords[((face[i])[1] - 1) * 3 + 2],
				textureCoords[((face[i])[4] - 1) * 2], textureCoords[((face[i])[4] - 1) * 2 + 1],
				normalCoords[((face[i])[7] - 1) * 3], normalCoords[((face[i])[7] - 1) * 3 + 1], normalCoords[((face[i])[7] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 1] = addVertex(((face[i])[1] - 1), &vertex2[0], 8);

			float vertex3[] = { vertexCoords[((face[i])[2] - 1) * 3], vertexCoords[((face[i])[2] - 1) * 3 + 1], vertexCoords[((face[i])[2] - 1) * 3 + 2],
				textureCoords[((face[i])[5] - 1) * 2], textureCoords[((face[i])[5] - 1) * 2 + 1],
				normalCoords[((face[i])[8] - 1) * 3], normalCoords[((face[i])[8] - 1) * 3 + 1], normalCoords[((face[i])[8] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 2] = addVertex(((face[i])[2] - 1), &vertex3[0], 8);

			numberOfTriangle++;
		}
	}else if (!normalCoords.empty()){

		m_hasNormals = true;
		m_indexBuffer.resize(m_numberOfTriangles * 3);

		int numberOfTriangle = 0;

		for (int i = 0; i < m_numberOfTriangles; i++){

			float vertex1[] = { vertexCoords[((face[i])[0] - 1) * 3], vertexCoords[((face[i])[0] - 1) * 3 + 1], vertexCoords[((face[i])[0] - 1) * 3 + 2],
				normalCoords[((face[i])[6] - 1) * 3], normalCoords[((face[i])[6] - 1) * 3 + 1], normalCoords[((face[i])[6] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3] = addVertex(((face[i])[0] - 1), &vertex1[0], 6);

			float vertex2[] = { vertexCoords[((face[i])[1] - 1) * 3], vertexCoords[((face[i])[1] - 1) * 3 + 1], vertexCoords[((face[i])[1] - 1) * 3 + 2],
				normalCoords[((face[i])[7] - 1) * 3], normalCoords[((face[i])[7] - 1) * 3 + 1], normalCoords[((face[i])[7] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 1] = addVertex(((face[i])[1] - 1), &vertex2[0], 6);

			float vertex3[] = { vertexCoords[((face[i])[2] - 1) * 3], vertexCoords[((face[i])[2] - 1) * 3 + 1], vertexCoords[((face[i])[2] - 1) * 3 + 2],
				normalCoords[((face[i])[8] - 1) * 3], normalCoords[((face[i])[8] - 1) * 3 + 1], normalCoords[((face[i])[8] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 2] = addVertex(((face[i])[2] - 1), &vertex3[0], 6);

			numberOfTriangle++;
		}
	}
	else if (!textureCoords.empty()){

		m_hasTexels = true;
		m_indexBuffer.resize(m_numberOfTriangles * 3);

		int numberOfTriangle = 0;
		for (int i = 0; i < m_numberOfTriangles; i++){


			float vertex1[] = { vertexCoords[((face[i])[0] - 1) * 3], vertexCoords[((face[i])[0] - 1) * 3 + 1], vertexCoords[((face[i])[0] - 1) * 3 + 2],
				textureCoords[((face[i])[3] - 1) * 2], textureCoords[((face[i])[3] - 1) * 2 + 1] };
			m_indexBuffer[numberOfTriangle * 3] = addVertex(((face[i])[0] - 1), &vertex1[0], 5);

			float vertex2[] = { vertexCoords[((face[i])[1] - 1) * 3], vertexCoords[((face[i])[1] - 1) * 3 + 1], vertexCoords[((face[i])[1] - 1) * 3 + 2],
				textureCoords[((face[i])[4] - 1) * 2], textureCoords[((face[i])[4] - 1) * 2 + 1] };
			m_indexBuffer[numberOfTriangle * 3 + 1] = addVertex(((face[i])[1] - 1), &vertex2[0], 5);

			float vertex3[] = { vertexCoords[((face[i])[2] - 1) * 3], vertexCoords[((face[i])[2] - 1) * 3 + 1], vertexCoords[((face[i])[2] - 1) * 3 + 2],
				textureCoords[((face[i])[5] - 1) * 2], textureCoords[((face[i])[5] - 1) * 2 + 1] };
			m_indexBuffer[numberOfTriangle * 3 + 2] = addVertex(((face[i])[2] - 1), &vertex3[0], 5);

			numberOfTriangle++;
		}

	}else{

		m_indexBuffer.resize(m_numberOfTriangles * 3);

		int numberOfTriangle = 0;

		for (int i = 0; i < m_numberOfTriangles; i++){

			float vertex1[] = { vertexCoords[((face[i])[0] - 1) * 3], vertexCoords[((face[i])[0] - 1) * 3 + 1], vertexCoords[((face[i])[0] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3] = addVertex(((face[i])[0] - 1), &vertex1[0], 3);

			float vertex2[] = { vertexCoords[((face[i])[1] - 1) * 3], vertexCoords[((face[i])[1] - 1) * 3 + 1], vertexCoords[((face[i])[1] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 1] = addVertex(((face[i])[1] - 1), &vertex2[0], 3);

			float vertex3[] = { vertexCoords[((face[i])[2] - 1) * 3], vertexCoords[((face[i])[2] - 1) * 3 + 1], vertexCoords[((face[i])[2] - 1) * 3 + 2] };
			m_indexBuffer[numberOfTriangle * 3 + 2] = addVertex(((face[i])[2] - 1), &vertex3[0], 3);

			numberOfTriangle++;
		}

	}

	std::shared_ptr<Triangle> triangle;
	const unsigned int *pTriangle = 0;
	float *pVertex0 = 0;
	float *pVertex1 = 0;
	float *pVertex2 = 0;

	if (m_hasNormals && m_hasTexels){

		int start = 0;
		int end = meshes[0]->m_numberOfTriangles;

		for (int j = 0; j < m_numberOfMeshes; j++){

			if (j > 0){

				start = end;
				end = end + meshes[j]->m_numberOfTriangles;
			}

			for (int i = start; i < end; i++){

				pTriangle = &m_indexBuffer[i * 3];

				pVertex0 = &m_vertexBuffer[pTriangle[0] * 8];
				pVertex1 = &m_vertexBuffer[pTriangle[1] * 8];
				pVertex2 = &m_vertexBuffer[pTriangle[2] * 8];

				meshes[j]->m_xmin = min(pVertex0[0], min(pVertex1[0], min(pVertex2[0], meshes[j]->m_xmin)));
				meshes[j]->m_ymin = min(pVertex0[1], min(pVertex1[1], min(pVertex2[1], meshes[j]->m_ymin)));
				meshes[j]->m_zmin = min(pVertex0[2], min(pVertex1[2], min(pVertex2[2], meshes[j]->m_zmin)));

				meshes[j]->m_xmax = max(pVertex0[0], max(pVertex1[0], max(pVertex2[0], meshes[j]->m_xmax)));
				meshes[j]->m_ymax = max(pVertex0[1], max(pVertex1[1], max(pVertex2[1], meshes[j]->m_ymax)));
				meshes[j]->m_zmax = max(pVertex0[2], max(pVertex1[2], max(pVertex2[2], meshes[j]->m_zmax)));

				Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
				Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
				Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, cull, smooth));
				triangle->setColor(meshes[j]->m_color);
				triangle->m_texture = meshes[j]->m_texture;
				triangle->m_material = meshes[j]->m_material;

				Vector2f uv1 = Vector2f(pVertex0[3], pVertex0[4]);
				Vector2f uv2 = Vector2f(pVertex1[3], pVertex1[4]);
				Vector2f uv3 = Vector2f(pVertex2[3], pVertex2[4]);

				triangle->setUV(uv1, uv2, uv3);

				Vector3f n1 = Vector3f(pVertex0[5], pVertex0[6], pVertex0[7]);
				Vector3f n2 = Vector3f(pVertex1[5], pVertex1[6], pVertex1[7]);
				Vector3f n3 = Vector3f(pVertex2[5], pVertex2[6], pVertex2[7]);

				triangle->setNormal(n1, n2, n3);
				
				meshes[j]->m_triangles.push_back(triangle);
			}

			xmin = min(meshes[j]->m_xmin, xmin);
			ymin = min(meshes[j]->m_ymin, ymin);
			zmin = min(meshes[j]->m_zmin, zmin);

			xmax = max(meshes[j]->m_xmax, xmax);
			ymax = max(meshes[j]->m_ymax, ymax);
			zmax = max(meshes[j]->m_zmax, zmax);

			
		}

	}else if (m_hasNormals){

		int start = 0;
		int end = meshes[0]->m_numberOfTriangles;

		for (int j = 0; j < m_numberOfMeshes; j++){

			if (j > 0){

				start = end;
				end = end + meshes[j]->m_numberOfTriangles;
			}

			for (int i = start; i < end; i++){

				pTriangle = &m_indexBuffer[i * 3];

				pVertex0 = &m_vertexBuffer[pTriangle[0] * 6];
				pVertex1 = &m_vertexBuffer[pTriangle[1] * 6];
				pVertex2 = &m_vertexBuffer[pTriangle[2] * 6];

				meshes[j]->m_xmin = min(pVertex0[0], min(pVertex1[0], min(pVertex2[0], meshes[j]->m_xmin)));
				meshes[j]->m_ymin = min(pVertex0[1], min(pVertex1[1], min(pVertex2[1], meshes[j]->m_ymin)));
				meshes[j]->m_zmin = min(pVertex0[2], min(pVertex1[2], min(pVertex2[2], meshes[j]->m_zmin)));

				meshes[j]->m_xmax = max(pVertex0[0], max(pVertex1[0], max(pVertex2[0], meshes[j]->m_xmax)));
				meshes[j]->m_ymax = max(pVertex0[1], max(pVertex1[1], max(pVertex2[1], meshes[j]->m_ymax)));
				meshes[j]->m_zmax = max(pVertex0[2], max(pVertex1[2], max(pVertex2[2], meshes[j]->m_zmax)));

				Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
				Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
				Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, cull, smooth));
				triangle->setColor(meshes[j]->m_color);
				triangle->m_texture = meshes[j]->m_texture;
				triangle->m_material = meshes[j]->m_material;

				Vector3f n1 = Vector3f(pVertex0[3], pVertex0[4], pVertex0[5]);
				Vector3f n2 = Vector3f(pVertex1[3], pVertex1[4], pVertex1[5]);
				Vector3f n3 = Vector3f(pVertex2[3], pVertex2[4], pVertex2[5]);

				triangle->setNormal(n1, n2, n3);

				meshes[j]->m_triangles.push_back(triangle);
			}

			xmin = min(meshes[j]->m_xmin, xmin);
			ymin = min(meshes[j]->m_ymin, ymin);
			zmin = min(meshes[j]->m_zmin, zmin);

			xmax = max(meshes[j]->m_xmax, xmax);
			ymax = max(meshes[j]->m_ymax, ymax);
			zmax = max(meshes[j]->m_zmax, zmax);
		}

	}else if (m_hasTexels){

		int start = 0;
		int end = meshes[0]->m_numberOfTriangles;

		for (int j = 0; j < m_numberOfMeshes; j++){

			if (j > 0){

				start = end;
				end = end + meshes[j]->m_numberOfTriangles;
			}

			for (int i = start; i < end; i++){

				pTriangle = &m_indexBuffer[i * 3];

				pVertex0 = &m_vertexBuffer[pTriangle[0] * 5];
				pVertex1 = &m_vertexBuffer[pTriangle[1] * 5];
				pVertex2 = &m_vertexBuffer[pTriangle[2] * 5];

				meshes[j]->m_xmin = min(pVertex0[0], min(pVertex1[0], min(pVertex2[0], meshes[j]->m_xmin)));
				meshes[j]->m_ymin = min(pVertex0[1], min(pVertex1[1], min(pVertex2[1], meshes[j]->m_ymin)));
				meshes[j]->m_zmin = min(pVertex0[2], min(pVertex1[2], min(pVertex2[2], meshes[j]->m_zmin)));

				meshes[j]->m_xmax = max(pVertex0[0], max(pVertex1[0], max(pVertex2[0], meshes[j]->m_xmax)));
				meshes[j]->m_ymax = max(pVertex0[1], max(pVertex1[1], max(pVertex2[1], meshes[j]->m_ymax)));
				meshes[j]->m_zmax = max(pVertex0[2], max(pVertex1[2], max(pVertex2[2], meshes[j]->m_zmax)));

				Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
				Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
				Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, cull, smooth));
				triangle->setColor(meshes[j]->m_color);
				triangle->m_texture = meshes[j]->m_texture;
				triangle->m_material = meshes[j]->m_material;

				Vector2f uv1 = Vector2f(pVertex0[3], pVertex0[4]);
				Vector2f uv2 = Vector2f(pVertex1[3], pVertex1[4]);
				Vector2f uv3 = Vector2f(pVertex2[3], pVertex2[4]);

				triangle->setUV(uv1, uv2, uv3);

				meshes[j]->m_triangles.push_back(triangle);
			}

			xmin = min(meshes[j]->m_xmin, xmin);
			ymin = min(meshes[j]->m_ymin, ymin);
			zmin = min(meshes[j]->m_zmin, zmin);

			xmax = max(meshes[j]->m_xmax, xmax);
			ymax = max(meshes[j]->m_ymax, ymax);
			zmax = max(meshes[j]->m_zmax, zmax);
		}
	}else{

		int start = 0;
		int end = meshes[0]->m_numberOfTriangles;

		for (int j = 0; j < m_numberOfMeshes; j++){

			if (j > 0){

				start = end;
				end = end + meshes[j]->m_numberOfTriangles;
			}

			for (int i = start; i < end; i++){

				pTriangle = &m_indexBuffer[i * 3];

				pVertex0 = &m_vertexBuffer[pTriangle[0] * 3];
				pVertex1 = &m_vertexBuffer[pTriangle[1] * 3];
				pVertex2 = &m_vertexBuffer[pTriangle[2] * 3];

				meshes[j]->m_xmin = min(pVertex0[0], min(pVertex1[0], min(pVertex2[0], meshes[j]->m_xmin)));
				meshes[j]->m_ymin = min(pVertex0[1], min(pVertex1[1], min(pVertex2[1], meshes[j]->m_ymin)));
				meshes[j]->m_zmin = min(pVertex0[2], min(pVertex1[2], min(pVertex2[2], meshes[j]->m_zmin)));

				meshes[j]->m_xmax = max(pVertex0[0], max(pVertex1[0], max(pVertex2[0], meshes[j]->m_xmax)));
				meshes[j]->m_ymax = max(pVertex0[1], max(pVertex1[1], max(pVertex2[1], meshes[j]->m_ymax)));
				meshes[j]->m_zmax = max(pVertex0[2], max(pVertex1[2], max(pVertex2[2], meshes[j]->m_zmax)));

				Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
				Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
				Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, cull, smooth));
				triangle->setColor(meshes[j]->m_color);
				triangle->m_texture = meshes[j]->m_texture;
				triangle->m_material = meshes[j]->m_material;

				meshes[j]->m_triangles.push_back(triangle);
			}

			xmin = min(meshes[j]->m_xmin, xmin);
			ymin = min(meshes[j]->m_ymin, ymin);
			zmin = min(meshes[j]->m_zmin, zmin);

			xmax = max(meshes[j]->m_xmax, xmax);
			ymax = max(meshes[j]->m_ymax, ymax);
			zmax = max(meshes[j]->m_zmax, zmax);

		}

	}

	
	std::cout << "Number of faces: " << m_numberOfTriangles << std::endl;
	std::cout << "Number of Meshes: " << m_numberOfMeshes << std::endl;

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);
	box = BBox(p1, p2 - p1);
	ModelIndexed::bounds = true;

	//calcBounds();

	return true;

}

int ModelIndexed::addVertex(int hash, float *pVertex, int stride){

	int index = -1;
	std::map<int, std::vector<int> >::const_iterator iter = m_vertexCache.find(hash);

	if (iter == m_vertexCache.end()){
		// Vertex hash doesn't exist in the cache.
		index = static_cast<int>(m_vertexBuffer.size() / stride);
		m_vertexBuffer.insert(m_vertexBuffer.end(), pVertex, pVertex + stride);
		m_vertexCache.insert(std::make_pair(hash, std::vector<int>(1, index)));

	}
	else{
		// One or more vertices have been hashed to this entry in the cache.
		const std::vector<int> &vertices = iter->second;
		const float *pCachedVertex = 0;
		bool found = false;

		for (std::vector<int>::const_iterator i = vertices.begin(); i != vertices.end(); ++i){
			index = *i;
			pCachedVertex = &m_vertexBuffer[index * 3];



			if (memcmp(pCachedVertex, pVertex, sizeof(float)* stride) == 0){
				found = true;
				break;
			}
		}

		if (!found){
			index = static_cast<int>(m_vertexBuffer.size() / stride);
			m_vertexBuffer.insert(m_vertexBuffer.end(), pVertex, pVertex + stride);
			m_vertexCache[hash].push_back(index);
		}
	}

	return index;
}


void ModelIndexed::generateNormals(){

	if (m_hasNormals) { return; }

	std::vector<float> tmpVertex;

	const unsigned int *pTriangle = 0;
	float *pVertex0 = 0;
	float *pVertex1 = 0;
	float *pVertex2 = 0;
	float edge1[3] = { 0.0f, 0.0f, 0.0f };
	float edge2[3] = { 0.0f, 0.0f, 0.0f };
	float normal[3] = { 0.0f, 0.0f, 0.0f };
	float length = 0.0f;
	int modulo = m_hasTexels ? 5 : 3;
	int vertexLength = m_hasTexels ? 8 : 6;
	int vertexOffset = m_hasTexels ? 2 : 0;
	int totalTriangles = m_numberOfTriangles;

	for (int i = 0; i < m_vertexBuffer.size(); i++){

		tmpVertex.push_back(m_vertexBuffer[i]);

		if ((i + 1) % modulo == 0){

			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
		}
	}

	for (int i = 0; i < totalTriangles; i++){

		pTriangle = &m_indexBuffer[i * 3];

		pVertex0 = &m_vertexBuffer[pTriangle[0] * modulo];
		pVertex1 = &m_vertexBuffer[pTriangle[1] * modulo];
		pVertex2 = &m_vertexBuffer[pTriangle[2] * modulo];

		// Calculate triangle face normal.
		edge1[0] = pVertex1[0] - pVertex0[0];
		edge1[1] = pVertex1[1] - pVertex0[1];
		edge1[2] = pVertex1[2] - pVertex0[2];

		edge2[0] = pVertex2[0] - pVertex0[0];
		edge2[1] = pVertex2[1] - pVertex0[1];
		edge2[2] = pVertex2[2] - pVertex0[2];

		normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
		normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
		normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

		// Accumulate the normals.
		tmpVertex[pTriangle[0] * vertexLength + 3 + vertexOffset] = tmpVertex[pTriangle[0] * vertexLength + 3 + vertexOffset] + normal[0];
		tmpVertex[pTriangle[0] * vertexLength + 4 + vertexOffset] = tmpVertex[pTriangle[0] * vertexLength + 4 + vertexOffset] + normal[1];
		tmpVertex[pTriangle[0] * vertexLength + 5 + vertexOffset] = tmpVertex[pTriangle[0] * vertexLength + 5 + vertexOffset] + normal[2];

		tmpVertex[pTriangle[1] * vertexLength + 3 + vertexOffset] = tmpVertex[pTriangle[1] * vertexLength + 3 + vertexOffset] + normal[0];
		tmpVertex[pTriangle[1] * vertexLength + 4 + vertexOffset] = tmpVertex[pTriangle[1] * vertexLength + 4 + vertexOffset] + normal[1];
		tmpVertex[pTriangle[1] * vertexLength + 5 + vertexOffset] = tmpVertex[pTriangle[1] * vertexLength + 5 + vertexOffset] + normal[2];

		tmpVertex[pTriangle[2] * vertexLength + 3 + vertexOffset] = tmpVertex[pTriangle[2] * vertexLength + 3 + vertexOffset] + normal[0];
		tmpVertex[pTriangle[2] * vertexLength + 4 + vertexOffset] = tmpVertex[pTriangle[2] * vertexLength + 4 + vertexOffset] + normal[1];
		tmpVertex[pTriangle[2] * vertexLength + 5 + vertexOffset] = tmpVertex[pTriangle[2] * vertexLength + 5 + vertexOffset] + normal[2];
	}

	// Normalize the vertex normals.
	for (int i = 0; i < tmpVertex.size(); i = i + vertexLength){

		float length = 1.0f / sqrtf(tmpVertex[i + 3 + vertexOffset] * tmpVertex[i + 3 + vertexOffset] +
			tmpVertex[i + 4 + vertexOffset] * tmpVertex[i + 4 + vertexOffset] +
			tmpVertex[i + 5 + vertexOffset] * tmpVertex[i + 5 + vertexOffset]);

		tmpVertex[i + 3 + vertexOffset] *= length;
		tmpVertex[i + 4 + vertexOffset] *= length;
		tmpVertex[i + 5 + vertexOffset] *= length;
	}

	m_vertexBuffer.clear();
	copy(tmpVertex.begin(), tmpVertex.end(), back_inserter(m_vertexBuffer));
	tmpVertex.clear();

	int start = 0;
	int end = meshes[0]->m_numberOfTriangles;

	for (int j = 0; j < m_numberOfMeshes; j++){


		if (j > 0){

			start = end;
			end = end + meshes[j]->m_numberOfTriangles;
		}

		std::shared_ptr<Triangle> triangle;

		meshes[j]->m_triangles.clear();
		for (int i = start; i < end; i++){

			pTriangle = &m_indexBuffer[i * 3];

			pVertex0 = &m_vertexBuffer[pTriangle[0] * vertexLength];
			pVertex1 = &m_vertexBuffer[pTriangle[1] * vertexLength];
			pVertex2 = &m_vertexBuffer[pTriangle[2] * vertexLength];


			Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
			Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
			Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

			triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, true, true));
			triangle->setColor(meshes[j]->m_color);
			triangle->m_texture = meshes[j]->m_texture;
			triangle->m_material = meshes[j]->m_material;

			if (m_hasTexels){

				Vector2f uv1 = Vector2f(pVertex0[3], pVertex0[4]);
				Vector2f uv2 = Vector2f(pVertex1[3], pVertex1[4]);
				Vector2f uv3 = Vector2f(pVertex2[3], pVertex2[4]);

				triangle->setUV(uv1, uv2, uv3);

				Vector3f n1 = Vector3f(pVertex0[5], pVertex0[6], pVertex0[7]);
				Vector3f n2 = Vector3f(pVertex1[5], pVertex1[6], pVertex1[7]);
				Vector3f n3 = Vector3f(pVertex2[5], pVertex2[6], pVertex2[7]);

				triangle->setNormal(n1, n2, n3);

			}else{

				Vector3f n1 = Vector3f(pVertex0[3], pVertex0[4], pVertex0[5]);
				Vector3f n2 = Vector3f(pVertex1[3], pVertex1[4], pVertex1[5]);
				Vector3f n3 = Vector3f(pVertex2[3], pVertex2[4], pVertex2[5]);

				triangle->setNormal(n1, n2, n3);
			}

			meshes[j]->m_triangles.push_back(triangle);

		}
	}

	m_hasNormals = true;
}

void ModelIndexed::generateTangents(){

	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;
	}

	std::vector<float> tmpVertex;
	const unsigned int *pTriangle = 0;
	float *pVertex0 = 0;
	float *pVertex1 = 0;
	float *pVertex2 = 0;
	float edge1[3] = { 0.0f, 0.0f, 0.0f };
	float edge2[3] = { 0.0f, 0.0f, 0.0f };
	float texEdge1[2] = { 0.0f, 0.0f };
	float texEdge2[2] = { 0.0f, 0.0f };
	float tangent[3] = { 0.0f, 0.0f, 0.0f };
	float bitangent[3] = { 0.0f, 0.0f, 0.0f };
	float det = 0.0f;
	float nDotT = 0.0f;
	float bDotB = 0.0f;
	float length = 0.0f;

	int totalVertices = m_vertexBuffer.size() / 8;
	int totalTriangles = m_numberOfTriangles;

	for (int i = 0; i < m_vertexBuffer.size(); i++){

		tmpVertex.push_back(m_vertexBuffer[i]);

		if ((i + 1) % 8 == 0){

			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
		}
	}


	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < totalTriangles; ++i){

		pTriangle = &m_indexBuffer[i * 3];

		pVertex0 = &m_vertexBuffer[pTriangle[0] * 8];
		pVertex1 = &m_vertexBuffer[pTriangle[1] * 8];
		pVertex2 = &m_vertexBuffer[pTriangle[2] * 8];

		// Calculate the triangle face tangent and bitangent.

		edge1[0] = pVertex1[0] - pVertex0[0];
		edge1[1] = pVertex1[1] - pVertex0[1];
		edge1[2] = pVertex1[2] - pVertex0[2];

		edge2[0] = pVertex2[0] - pVertex0[0];
		edge2[1] = pVertex2[1] - pVertex0[1];
		edge2[2] = pVertex2[2] - pVertex0[2];

		texEdge1[0] = pVertex1[3] - pVertex0[3];
		texEdge1[1] = pVertex1[4] - pVertex0[4];

		texEdge2[0] = pVertex2[3] - pVertex0[3];
		texEdge2[1] = pVertex2[4] - pVertex0[4];

		det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

		if (fabs(det) < 1e-6f){

			tangent[0] = 1.0f;
			tangent[1] = 0.0f;
			tangent[2] = 0.0f;

			bitangent[0] = 0.0f;
			bitangent[1] = 1.0f;
			bitangent[2] = 0.0f;

		}else{

			det = 1.0f / det;

			tangent[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
			tangent[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[1]) * det;
			tangent[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[2]) * det;

			bitangent[0] = (texEdge1[0] * edge2[0] - texEdge2[0] * edge1[0]) * det;
			bitangent[1] = (texEdge1[0] * edge2[1] - texEdge2[0] * edge1[1]) * det;
			bitangent[2] = (texEdge1[0] * edge2[2] - texEdge2[0] * edge1[2]) * det;
		}


		// Accumulate the tangents and bitangents.
		tmpVertex[pTriangle[0] * 15 + 8] = tmpVertex[pTriangle[0] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[0] * 15 + 9] = tmpVertex[pTriangle[0] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[0] * 15 + 10] = tmpVertex[pTriangle[0] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[0] * 15 + 12] = tmpVertex[pTriangle[0] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[0] * 15 + 13] = tmpVertex[pTriangle[0] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[0] * 15 + 14] = tmpVertex[pTriangle[0] * 15 + 14] + bitangent[2];

		tmpVertex[pTriangle[1] * 15 + 8] = tmpVertex[pTriangle[1] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[1] * 15 + 9] = tmpVertex[pTriangle[1] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[1] * 15 + 10] = tmpVertex[pTriangle[1] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[1] * 15 + 12] = tmpVertex[pTriangle[1] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[1] * 15 + 13] = tmpVertex[pTriangle[1] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[1] * 15 + 14] = tmpVertex[pTriangle[1] * 15 + 14] + bitangent[2];

		tmpVertex[pTriangle[2] * 15 + 8] = tmpVertex[pTriangle[2] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[2] * 15 + 9] = tmpVertex[pTriangle[2] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[2] * 15 + 10] = tmpVertex[pTriangle[2] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[2] * 15 + 12] = tmpVertex[pTriangle[2] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[2] * 15 + 13] = tmpVertex[pTriangle[2] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[2] * 15 + 14] = tmpVertex[pTriangle[2] * 15 + 14] + bitangent[2];
	}

	// Orthogonalize and normalize the vertex tangents.
	for (int i = 0; i < tmpVertex.size(); i = i + 15){

		pVertex0 = &tmpVertex[i];

		// Gram-Schmidt orthogonalize tangent with normal.

		nDotT = pVertex0[5] * pVertex0[8] +
			pVertex0[6] * pVertex0[9] +
			pVertex0[7] * pVertex0[10];

		pVertex0[8] -= pVertex0[5] * nDotT;
		pVertex0[9] -= pVertex0[6] * nDotT;
		pVertex0[10] -= pVertex0[7] * nDotT;

		// Normalize the tangent.

		length = 1.0f / sqrtf(pVertex0[8] * pVertex0[8] +
			pVertex0[9] * pVertex0[9] +
			pVertex0[10] * pVertex0[10]);

		pVertex0[8] *= length;
		pVertex0[9] *= length;
		pVertex0[10] *= length;

		bitangent[0] = (pVertex0[6] * pVertex0[10]) -
			(pVertex0[7] * pVertex0[9]);
		bitangent[1] = (pVertex0[7] * pVertex0[8]) -
			(pVertex0[5] * pVertex0[10]);
		bitangent[2] = (pVertex0[5] * pVertex0[9]) -
			(pVertex0[6] * pVertex0[8]);

		bDotB = bitangent[0] * pVertex0[12] +
			bitangent[1] * pVertex0[13] +
			bitangent[2] * pVertex0[14];

		// Calculate handedness
		pVertex0[11] = (bDotB < 0.0f) ? 1.0f : -1.0f;

		if (bDotB < 0.0f){
			pVertex0[8] = -pVertex0[8];
			pVertex0[9] = -pVertex0[9];
			pVertex0[10] = -pVertex0[10];
		}

		pVertex0[12] = bitangent[0];
		pVertex0[13] = bitangent[1];
		pVertex0[14] = bitangent[2];
	}

	m_vertexBuffer.clear();
	copy(tmpVertex.begin(), tmpVertex.end(), back_inserter(m_vertexBuffer));
	tmpVertex.clear();

	int start = 0;
	int end = meshes[0]->m_numberOfTriangles;

	for (int j = 0; j < m_numberOfMeshes; j++){

		if (j > 0){
			start = end;
			end = end + meshes[j]->m_numberOfTriangles;
		}

		std::shared_ptr<Triangle> triangle;

		meshes[j]->m_triangles.clear();
		for (int i = start; i < end; i++){

			pTriangle = &m_indexBuffer[i * 3];

			pVertex0 = &m_vertexBuffer[pTriangle[0] * 15];
			pVertex1 = &m_vertexBuffer[pTriangle[1] * 15];
			pVertex2 = &m_vertexBuffer[pTriangle[2] * 15];

			Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
			Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
			Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

			triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, true, true));
			triangle->setColor(meshes[j]->m_color);
			triangle->m_texture = meshes[j]->m_texture;
			triangle->m_material = meshes[j]->m_material;

			Vector2f uv1 = Vector2f(pVertex0[3], pVertex0[4]);
			Vector2f uv2 = Vector2f(pVertex1[3], pVertex1[4]);
			Vector2f uv3 = Vector2f(pVertex2[3], pVertex2[4]);

			triangle->setUV(uv1, uv2, uv3);

			Vector3f n1 = Vector3f(pVertex0[5], pVertex0[6], pVertex0[7]);
			Vector3f n2 = Vector3f(pVertex1[5], pVertex1[6], pVertex1[7]);
			Vector3f n3 = Vector3f(pVertex2[5], pVertex2[6], pVertex2[7]);

			triangle->setNormal(n1, n2, n3);

			Vector4f t1 = Vector4f(pVertex0[8], pVertex0[9], pVertex0[10], pVertex0[11]);
			Vector4f t2 = Vector4f(pVertex1[8], pVertex1[9], pVertex1[10], pVertex1[11]);
			Vector4f t3 = Vector4f(pVertex2[8], pVertex2[9], pVertex2[10], pVertex2[11]);

			triangle->setTangents(t1, t2, t3);

			Vector3f bt1 = Vector3f(pVertex0[12], pVertex0[13], pVertex0[14]);
			Vector3f bt2 = Vector3f(pVertex1[12], pVertex1[13], pVertex1[14]);
			Vector3f bt3 = Vector3f(pVertex2[12], pVertex2[13], pVertex2[14]);

			triangle->setBiTangents(bt1, bt2, bt3);

			meshes[j]->m_triangles.push_back(triangle);
		}
	}

	
	m_hasTangents = true;
}


void ModelIndexed::generateNormalDerivatives(){

	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;
	}

	std::vector<float> tmpVertex;
	const unsigned int *pTriangle = 0;
	float *pVertex0 = 0;
	float *pVertex1 = 0;
	float *pVertex2 = 0;
	float edge1[3] = { 0.0f, 0.0f, 0.0f };
	float edge2[3] = { 0.0f, 0.0f, 0.0f };
	float texEdge1[2] = { 0.0f, 0.0f };
	float texEdge2[2] = { 0.0f, 0.0f };
	float tangent[3] = { 0.0f, 0.0f, 0.0f };
	float bitangent[3] = { 0.0f, 0.0f, 0.0f };
	float det = 0.0f;
	float nDotT = 0.0f;
	float bDotB = 0.0f;
	float length = 0.0f;

	int totalVertices = m_vertexBuffer.size() / 8;
	int totalTriangles = m_numberOfTriangles;

	for (int i = 0; i < m_vertexBuffer.size(); i++){

		tmpVertex.push_back(m_vertexBuffer[i]);

		if ((i + 1) % 8 == 0){

			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
			tmpVertex.push_back(0.0);
		}
	}


	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < totalTriangles; ++i){

		pTriangle = &m_indexBuffer[i * 3];

		pVertex0 = &m_vertexBuffer[pTriangle[0] * 8];
		pVertex1 = &m_vertexBuffer[pTriangle[1] * 8];
		pVertex2 = &m_vertexBuffer[pTriangle[2] * 8];

		// Calculate the triangle face tangent and bitangent.

		edge1[0] = pVertex1[5] - pVertex0[5];
		edge1[1] = pVertex1[6] - pVertex0[6];
		edge1[2] = pVertex1[7] - pVertex0[7];

		edge2[0] = pVertex2[5] - pVertex0[5];
		edge2[1] = pVertex2[6] - pVertex0[6];
		edge2[2] = pVertex2[7] - pVertex0[7];

		texEdge1[0] = pVertex1[3] - pVertex0[3];
		texEdge1[1] = pVertex1[4] - pVertex0[4];

		texEdge2[0] = pVertex2[3] - pVertex0[3];
		texEdge2[1] = pVertex2[4] - pVertex0[4];

		det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

		if (fabs(det) < 1e-6f){

			tangent[0] = 1.0f;
			tangent[1] = 0.0f;
			tangent[2] = 0.0f;

			bitangent[0] = 0.0f;
			bitangent[1] = 1.0f;
			bitangent[2] = 0.0f;

		}else{

			det = 1.0f / det;

			tangent[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
			tangent[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[1]) * det;
			tangent[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[2]) * det;

			bitangent[0] = (texEdge1[0] * edge2[0] - texEdge2[0] * edge1[0]) * det;
			bitangent[1] = (texEdge1[0] * edge2[1] - texEdge2[0] * edge1[1]) * det;
			bitangent[2] = (texEdge1[0] * edge2[2] - texEdge2[0] * edge1[2]) * det;
		}


		// Accumulate the tangents and bitangents.
		tmpVertex[pTriangle[0] * 15 + 8] = tmpVertex[pTriangle[0] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[0] * 15 + 9] = tmpVertex[pTriangle[0] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[0] * 15 + 10] = tmpVertex[pTriangle[0] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[0] * 15 + 12] = tmpVertex[pTriangle[0] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[0] * 15 + 13] = tmpVertex[pTriangle[0] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[0] * 15 + 14] = tmpVertex[pTriangle[0] * 15 + 14] + bitangent[2];

		tmpVertex[pTriangle[1] * 15 + 8] = tmpVertex[pTriangle[1] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[1] * 15 + 9] = tmpVertex[pTriangle[1] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[1] * 15 + 10] = tmpVertex[pTriangle[1] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[1] * 15 + 12] = tmpVertex[pTriangle[1] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[1] * 15 + 13] = tmpVertex[pTriangle[1] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[1] * 15 + 14] = tmpVertex[pTriangle[1] * 15 + 14] + bitangent[2];

		tmpVertex[pTriangle[2] * 15 + 8] = tmpVertex[pTriangle[2] * 15 + 8] + tangent[0];
		tmpVertex[pTriangle[2] * 15 + 9] = tmpVertex[pTriangle[2] * 15 + 9] + tangent[1];
		tmpVertex[pTriangle[2] * 15 + 10] = tmpVertex[pTriangle[2] * 15 + 10] + tangent[2];

		tmpVertex[pTriangle[2] * 15 + 12] = tmpVertex[pTriangle[2] * 15 + 12] + bitangent[0];
		tmpVertex[pTriangle[2] * 15 + 13] = tmpVertex[pTriangle[2] * 15 + 13] + bitangent[1];
		tmpVertex[pTriangle[2] * 15 + 14] = tmpVertex[pTriangle[2] * 15 + 14] + bitangent[2];
	}

	// Orthogonalize and normalize the vertex tangents.
	/*for (int i = 0; i < tmpVertex.size(); i = i + 15){

		pVertex0 = &tmpVertex[i];

		// Gram-Schmidt orthogonalize tangent with normal.

		nDotT = pVertex0[5] * pVertex0[8] +
			pVertex0[6] * pVertex0[9] +
			pVertex0[7] * pVertex0[10];

		pVertex0[8] -= pVertex0[5] * nDotT;
		pVertex0[9] -= pVertex0[6] * nDotT;
		pVertex0[10] -= pVertex0[7] * nDotT;

		// Normalize the tangent.

		length = 1.0f / sqrtf(pVertex0[8] * pVertex0[8] +
			pVertex0[9] * pVertex0[9] +
			pVertex0[10] * pVertex0[10]);

		pVertex0[8] *= length;
		pVertex0[9] *= length;
		pVertex0[10] *= length;

		bitangent[0] = (pVertex0[6] * pVertex0[10]) -
			(pVertex0[7] * pVertex0[9]);
		bitangent[1] = (pVertex0[7] * pVertex0[8]) -
			(pVertex0[5] * pVertex0[10]);
		bitangent[2] = (pVertex0[5] * pVertex0[9]) -
			(pVertex0[6] * pVertex0[8]);

		bDotB = bitangent[0] * pVertex0[12] +
			bitangent[1] * pVertex0[13] +
			bitangent[2] * pVertex0[14];

		// Calculate handedness
		pVertex0[11] = (bDotB < 0.0f) ? 1.0f : -1.0f;

		if (bDotB < 0.0f){
			pVertex0[8] = -pVertex0[8];
			pVertex0[9] = -pVertex0[9];
			pVertex0[10] = -pVertex0[10];
		}

		pVertex0[12] = bitangent[0];
		pVertex0[13] = bitangent[1];
		pVertex0[14] = bitangent[2];
	}*/

	m_vertexBuffer.clear();
	copy(tmpVertex.begin(), tmpVertex.end(), back_inserter(m_vertexBuffer));
	tmpVertex.clear();

	int start = 0;
	int end = meshes[0]->m_numberOfTriangles;

	for (int j = 0; j < m_numberOfMeshes; j++){

		if (j > 0){
			start = end;
			end = end + meshes[j]->m_numberOfTriangles;
		}

		std::shared_ptr<Triangle> triangle;

		meshes[j]->m_triangles.clear();
		for (int i = start; i < end; i++){

			pTriangle = &m_indexBuffer[i * 3];

			pVertex0 = &m_vertexBuffer[pTriangle[0] * 15];
			pVertex1 = &m_vertexBuffer[pTriangle[1] * 15];
			pVertex2 = &m_vertexBuffer[pTriangle[2] * 15];

			Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
			Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
			Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

			triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, true, true));
			triangle->setColor(meshes[j]->m_color);
			triangle->m_texture = meshes[j]->m_texture;
			triangle->m_material = meshes[j]->m_material;

			Vector2f uv1 = Vector2f(pVertex0[3], pVertex0[4]);
			Vector2f uv2 = Vector2f(pVertex1[3], pVertex1[4]);
			Vector2f uv3 = Vector2f(pVertex2[3], pVertex2[4]);

			triangle->setUV(uv1, uv2, uv3);

			Vector3f n1 = Vector3f(pVertex0[5], pVertex0[6], pVertex0[7]);
			Vector3f n2 = Vector3f(pVertex1[5], pVertex1[6], pVertex1[7]);
			Vector3f n3 = Vector3f(pVertex2[5], pVertex2[6], pVertex2[7]);

			triangle->setNormal(n1, n2, n3);

			Vector4f t1 = Vector4f(pVertex0[8], pVertex0[9], pVertex0[10], pVertex0[11]).normalize();
			Vector4f t2 = Vector4f(pVertex1[8], pVertex1[9], pVertex1[10], pVertex1[11]).normalize();
			Vector4f t3 = Vector4f(pVertex2[8], pVertex2[9], pVertex2[10], pVertex2[11]).normalize();

			triangle->setTangents(t1, t2, t3);

			Vector3f bt1 = Vector3f(pVertex0[12], pVertex0[13], pVertex0[14]).normalize();
			Vector3f bt2 = Vector3f(pVertex1[12], pVertex1[13], pVertex1[14]).normalize();
			Vector3f bt3 = Vector3f(pVertex2[12], pVertex2[13], pVertex2[14]).normalize();

			triangle->setBiTangents(bt1, bt2, bt3);

			meshes[j]->m_triangles.push_back(triangle);
		}
	}


	m_hasTangents = true;
}

void ModelIndexed::buildKDTree(){
	
	std::cout << "Build KDTree!" << std::endl;

	for (int j = 0; j < m_numberOfMeshes; j++){

		m_triangles.insert(m_triangles.end(), meshes[j]->m_triangles.begin(), meshes[j]->m_triangles.end());
		meshes[j]->m_triangles.clear();
	}
	
	/*for (int i = 0; i < m_triangles.size(); i++){
	std::cout << m_triangles[i]->m_texture << std::endl;
	}*/

	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	std::cout << "Finished KDTree!" << std::endl;

}
////////////////////////////////////////////////////Mesh////////////////////////////////////

MeshIndexed::MeshIndexed(std::string mltName, int numberTriangles, ModelIndexed *model){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberOfTriangles = numberTriangles;
	m_mltName = mltName;
	m_texture = NULL;
	//m_normalMap = NULL;
	m_material = NULL;
	m_model = std::unique_ptr<ModelIndexed>(model);

	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;

	m_indexBuffer.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();

	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

MeshIndexed::MeshIndexed(int numberTriangles, ModelIndexed *model){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberOfTriangles = numberTriangles;
	m_texture = NULL;
	//m_normalMap = NULL;
	m_material = NULL;
	m_model = std::unique_ptr<ModelIndexed>(model);

	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;

	m_indexBuffer.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();

	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

MeshIndexed::~MeshIndexed(){

}

bool MeshIndexed::readMaterial(const char* filename){

	std::vector<std::string*>lines;
	int start = -1;
	int end = -1;

	std::ifstream in(filename);


	if (!in.is_open()){

		std::cout << "Mlt file not found" << std::endl;
		return false;
	}

	std::string line;
	while (getline(in, line)){
		lines.push_back(new std::string(line));

	}
	in.close();


	for (unsigned int i = 0; i < lines.size(); i++){

		if (strcmp((*lines[i]).c_str(), m_mltName.c_str()) == 0){

			start = i;
			continue;
		}


		if ((*lines[i]).find("newmtl") != std::string::npos && start > 0){

			end = i;
			break;
		}
		else {

			end = lines.size();
		}

	}

	if (start < 0 || end < 0) return false;

	for (int i = start; i < end; i++){

		if ((*lines[i])[0] == '#'){

			continue;

		}else if ((*lines[i])[0] == 'N' && (*lines[i])[1] == 's'){

			int tmp;
			sscanf(lines[i]->c_str(), "Ns %i", &tmp);
			//static_cast<std::shared_ptr<Phong>>(m_material.get())->m_shinies = tmp;
			m_material->m_shinies = tmp;


		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'a'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ka %f %f %f", &tmpx, &tmpy, &tmpz);



			m_material->m_ambient = Color(tmpx, tmpy, tmpz);
		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'd'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Kd %f %f %f", &tmpx, &tmpy, &tmpz);



			m_material->m_diffuse = Color(tmpx, tmpy, tmpz);

		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 's'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ks %f %f %f", &tmpx, &tmpy, &tmpz);



			m_material->m_specular = Color(tmpx, tmpy, tmpz);

		}else if ((*lines[i])[0] == 'm'){


			char identifierBuffer[20], valueBuffer[250];;
			sscanf(lines[i]->c_str(), "%s %s", identifierBuffer, valueBuffer);

			if (strstr(identifierBuffer, "map_Kd") != 0){

				m_material->colorMapPath = valueBuffer;


			}else if (strstr(identifierBuffer, "map_bump") != 0){

				m_material->bumpMapPath = valueBuffer;

			}

		}
	}



	for (unsigned int i = 0; i < lines.size(); i++){

		delete lines[i];
	}

	return true;
}
