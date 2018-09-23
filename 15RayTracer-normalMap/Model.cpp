#include "Model.h"
#include "KDTree.h"

Model::Model() :OrientablePrimitive() {

	m_hasMaterials = false;
	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;
	m_hasColor = false;
	

	m_texture = NULL;
	m_material = NULL;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}

Model::Model(const Color &color) :OrientablePrimitive(color) {

	m_hasMaterials = false;
	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;
	m_hasColor = true;
	

	m_color = color;
	m_texture = NULL;
	m_material = NULL;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}

Model::~Model(){

	
}


void Model::hit(const Ray& a_ray, Hit &hit){
	// find the nearest intersection
	m_KDTree->intersectRec(a_ray, hit);
	
}

void Model::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	Model::bounds = true;
}

std::pair <float, float> Model::getUV(const Vector3f& a_pos){

	return m_KDTree->m_primitive->getUV(a_pos);
}

Color Model::getColor(const Vector3f& a_pos){
	
	if (m_texture){
		
		m_KDTree->m_primitive->m_texture = m_texture;
		return m_KDTree->m_primitive->getColor(a_pos);

	// maybe the texture isn't at the path of the mlt file, then a nulltexure will created
	}else if (m_KDTree->m_primitive->m_texture && m_useTexture){
		
		return m_KDTree->m_primitive->getColor(a_pos);

	}else if(m_hasColor) {
		
		return m_color;
		
	}else{
		
		return m_KDTree->m_primitive->m_color;
	}

}

Vector3f  Model::getNormal(const Vector3f& a_pos){
	
	if (m_hasNormals){

		return ( m_KDTree->m_primitive->getNormal(a_pos) * invT).normalize();
	}
	
	return Vector3f(0.0, 0.0, 0.0);
	
}

Vector3f Model::getTangent(const Vector3f& a_pos){

	if (m_hasTangents){
		
		return (m_KDTree->m_primitive->getTangent(a_pos) * invT ).normalize();
	}

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Model::getBiTangent(const Vector3f& a_pos){
	
	if (m_hasTangents){

		return ( m_KDTree->m_primitive->getBiTangent(a_pos) * invT).normalize();
	}

	return Vector3f(0.0, 0.0, 0.0);
}

std::shared_ptr<Material>  Model::getMaterialMesh(){

	if (m_materialMesh){

		return m_materialMesh;
	}

	return std::shared_ptr<Material>(new Phong());
}

std::shared_ptr<Material> Model::getMaterial(){

	if (m_material){

		return m_material;

	}else{
		
		return m_KDTree->m_primitive->m_material;
	}
}

bool compare(const std::array<int, 10> &i_lhs, const std::array<int, 10> &i_rhs){

	return i_lhs[9] < i_rhs[9];
}

bool Model::loadObject(const char* filename, bool cull, bool smooth){

	return loadObject(filename, Vector3f(0.0, 0.0, 1.0), 0.0, Vector3f(0.0, 0.0, 0.0), 1.0, cull, smooth);
}

bool Model::loadObject(const char* a_filename, Vector3f &rotate, float degree, Vector3f &translate, float scale, bool cull, bool smooth){

	std::string filename(a_filename);

	const size_t index = filename.rfind('/');

	if (std::string::npos != index){

		m_modelDirectory = filename.substr(0, index);
	}


	std::vector<std::array<int, 10>> face;
	std::map<std::string, int> name;
	std::vector <float> tmpVertexBuffer;

	int countMesh = 0;
	int assign = 0;
	char buffer[250];


	FILE * pFile = fopen(a_filename, "r");
	if (pFile == NULL){
		std::cout << "File not found" << std::endl;
		return false;
	}

	Matrix4f rotMtx;
	rotMtx.rotate(rotate, degree);

	int tmp = 0;

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

						  Vector3f position = Vector3f(tmpx, tmpy, tmpz);
						  position = rotMtx * position;

						  m_positions.push_back(Vector3f((position[0] * scale) + translate[0], (position[1] * scale) + translate[1], (position[2] * scale) + translate[2]));

						  break;

			}case 't':{

						  float tmpu, tmpv;



						  fgets(buffer, sizeof(buffer), pFile);
						  sscanf(buffer, "%f %f", &tmpu, &tmpv);
						  m_texels.push_back(Vector2f(tmpu, tmpv));

						  break;

			}case 'n':{

				float tmpx, tmpy, tmpz;
				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

				Vector3f normal = Vector3f(tmpx, tmpy, tmpz);
				Matrix4f rotMtx;
				rotMtx.rotate(rotate, degree);
				normal = rotMtx * normal;
				m_normals.push_back(Vector3f(normal[0], normal[1], normal[2]));
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


				}
				else{
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

			if (!m_texels.empty() && !m_normals.empty()){
				sscanf(buffer, "%d/%d/%d %d/%d/%d %d/%d/%d ", &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);
				face.push_back({ { a, b, c, t1, t2, t3, n1, n2, n3, assign } });

			}
			else if (!m_normals.empty()){
				sscanf(buffer, "%d//%d %d//%d %d//%d", &a, &n1, &b, &n2, &c, &n3);
				face.push_back({ { a, b, c, 0, 0, 0, n1, n2, n3, assign } });

			}
			else if (!m_texels.empty()){
				sscanf(buffer, "%d/%d %d/%d %d/%d", &a, &t1, &b, &t2, &c, &t3);
				face.push_back({ { a, b, c, t1, t2, t3, 0, 0, 0, assign } });

			}
			else {
				sscanf(buffer, "%d %d %d", &a, &b, &c);
				face.push_back({ { a, b, c, 0, 0, 0, 0, 0, 0, assign } });
			}
			break;

		}default: {

			break;
		}
		}

	}



	std::sort(face.begin(), face.end(), compare);

	std::map<int, int> dup;

	for (unsigned int i = 0; i < face.size(); i++){
		dup[face[i][9]]++;
	}



	m_numberOfMeshes = dup.size();
	m_numberOfVertices = m_positions.size();
	m_numberOfTriangles = face.size();

	std::map<int, int>::const_iterator iterDup = dup.begin();

	for (iterDup; iterDup != dup.end(); iterDup++){


		if (name.empty()){

			meshes.push_back(std::shared_ptr<Mesh>(new Mesh(iterDup->second, this)));

		}
		else{

			std::map<std::string, int >::const_iterator iterName = name.begin();
			for (iterName; iterName != name.end(); iterName++){

				if (iterDup->first == iterName->second){

					meshes.push_back(std::shared_ptr<Mesh>(new Mesh("newmtl " + iterName->first, iterDup->second, this)));
				}
			}

		}
	}

	dup.clear();
	name.clear();

	for (int j = 0; j < m_numberOfMeshes; j++){

		if (m_material){

			meshes[j]->m_material = std::shared_ptr<Material>(m_material);

		}
		else if (m_hasMaterials){

			meshes[j]->m_material = std::shared_ptr<Material>(new Phong());

		}

		if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){

			if (meshes[j]->m_material->colorMapPath != ""){

				meshes[j]->m_texture = std::shared_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]));

			}

			if (meshes[j]->m_material->bumpMapPath != ""){

				meshes[j]->m_material = static_cast<std::shared_ptr<NormalMap>>(new NormalMap(meshes[j]->m_material));
				static_cast<NormalMap*>(meshes[j]->m_material.get())->setNormalMap(std::shared_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->bumpMapPath)[0])));

			}

		}

		meshes[j]->m_color = Color(1.0f / (j + 1), 1.0f / (j + 1), 1.0f / (j + 1));

	}// end for
	m_materialMesh = meshes[0]->m_material;

	int start = 0;
	int end = meshes[0]->m_numberOfTriangles;

	for (int j = 0; j < m_numberOfMeshes; j++){


	if (j > 0){

	start = end;
	end = end + meshes[j]->m_numberOfTriangles;
	}

	Vector3f a;
	Vector3f b;
	Vector3f c;
	std::shared_ptr<Triangle> triangle;

	for (int i = start; i < end; i++){



	a = m_positions[(face[i])[0] - 1];
	b = m_positions[(face[i])[1] - 1];
	c = m_positions[(face[i])[2] - 1];

	m_indexBufferPosition.push_back((face[i])[0] - 1);
	m_indexBufferPosition.push_back((face[i])[1] - 1);
	m_indexBufferPosition.push_back((face[i])[2] - 1);

	meshes[j]->m_indexBuffer.push_back((face[i])[0] - 1);
	meshes[j]->m_indexBuffer.push_back((face[i])[1] - 1);
	meshes[j]->m_indexBuffer.push_back((face[i])[2] - 1);


	meshes[j]->m_xmin = min(a[0], min(b[0], min(c[0], meshes[j]->m_xmin)));
	meshes[j]->m_ymin = min(a[1], min(b[1], min(c[1], meshes[j]->m_ymin)));
	meshes[j]->m_zmin = min(a[2], min(b[2], min(c[2], meshes[j]->m_zmin)));

	meshes[j]->m_xmax = max(a[0], max(b[0], max(c[0], meshes[j]->m_xmax)));
	meshes[j]->m_ymax = max(a[1], max(b[1], max(c[1], meshes[j]->m_ymax)));
	meshes[j]->m_zmax = max(a[2], max(b[2], max(c[2], meshes[j]->m_zmax)));

	triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, cull, smooth));
	triangle->m_texture = meshes[j]->m_texture;
	triangle->m_material = meshes[j]->m_material;


	if (m_texels.size() > 0){

	m_indexBufferTexel.push_back((face[i])[3] - 1);
	m_indexBufferTexel.push_back((face[i])[4] - 1);
	m_indexBufferTexel.push_back((face[i])[5] - 1);

	meshes[j]->m_indexBufferTexel.push_back((face[i])[3] - 1);
	meshes[j]->m_indexBufferTexel.push_back((face[i])[4] - 1);
	meshes[j]->m_indexBufferTexel.push_back((face[i])[5] - 1);

	meshes[j]->m_hasTexels = true;
	m_hasTexels = true;

	triangle->setUV(m_texels[(face[i])[3] - 1], m_texels[(face[i])[4] - 1], m_texels[(face[i])[5] - 1]);
	}


	if (m_normals.size() > 0){

	m_indexBufferNormal.push_back((face[i])[6] - 1);
	m_indexBufferNormal.push_back((face[i])[7] - 1);
	m_indexBufferNormal.push_back((face[i])[8] - 1);

	meshes[j]->m_indexBufferNormal.push_back((face[i])[6] - 1);
	meshes[j]->m_indexBufferNormal.push_back((face[i])[7] - 1);
	meshes[j]->m_indexBufferNormal.push_back((face[i])[8] - 1);

	meshes[j]->m_hasNormals = true;
	m_hasNormals = true;

	triangle->setNormal(m_normals[(face[i])[6] - 1], m_normals[(face[i])[7] - 1], m_normals[(face[i])[8] - 1]);
	}

	meshes[j]->m_triangles.push_back(triangle);
	}

		xmin = min(meshes[j]->m_xmin, xmin);
		ymin = min(meshes[j]->m_ymin, ymin);
		zmin = min(meshes[j]->m_zmin, zmin);

		xmax = max(meshes[j]->m_xmax, xmax);
		ymax = max(meshes[j]->m_ymax, ymax);
		zmax = max(meshes[j]->m_zmax, zmax);
	


	}

	 
	
	std::cout << "Number of faces: " << m_numberOfTriangles << std::endl;
	std::cout << "Number of Meshes: " << m_numberOfMeshes << std::endl;
	calcBounds();


	return true;
}


bool Model::loadObject2(const char* filename, bool cull, bool smooth){
	
	return loadObject2(filename, Vector3f(0.0, 0.0, 1.0), 0.0, Vector3f(0.0, 0.0, 0.0), 1.0, cull, smooth);
}



bool Model::loadObject2(const char* a_filename, Vector3f &axis, float degree, Vector3f &translate, float scale, bool cull, bool smooth){

	std::string filename(a_filename);

	const size_t index = filename.rfind('/');

	if (std::string::npos != index){

		m_modelDirectory = filename.substr(0, index);
	}


	std::vector<std::array<int, 10>> face;
	std::map<std::string, int> name;
	std::vector <float> tmpVertexBuffer;

	int countMesh = 0;
	int assign = 0;
	char buffer[250];


	FILE * pFile = fopen(a_filename, "r");
	if (pFile == NULL){
		std::cout << "File not found" << std::endl;
		return false;
	}

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degree);

	int tmp = 0;

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

						  Vector3f position = Vector3f(tmpx, tmpy, tmpz);
						  position = rotMtx * position;
						 
						  m_positions.push_back(Vector3f((position[0] * scale) + translate[0], (position[1] * scale) + translate[1], (position[2] * scale) + translate[2]));

						  break;

			}case 't':{

						  float tmpu, tmpv;



						  fgets(buffer, sizeof(buffer), pFile);
						  sscanf(buffer, "%f %f", &tmpu, &tmpv);
						  m_texels.push_back(Vector2f(tmpu, tmpv));

						  break;

			}case 'n':{

				float tmpx, tmpy, tmpz;
				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

				Vector3f normal = Vector3f(tmpx, tmpy, tmpz);
				Matrix4f rotMtx;
				rotMtx.rotate(axis, degree);
				normal = rotMtx * normal;
				m_normals.push_back(Vector3f(normal[0], normal[1], normal[2]));
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


				}
				else{
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

			if (!m_texels.empty() && !m_normals.empty()){
				sscanf(buffer, "%d/%d/%d %d/%d/%d %d/%d/%d ", &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);
				face.push_back({ { a, b, c, t1, t2, t3, n1, n2, n3, assign } });

			}
			else if (!m_normals.empty()){
				sscanf(buffer, "%d//%d %d//%d %d//%d", &a, &n1, &b, &n2, &c, &n3);
				face.push_back({ { a, b, c, 0, 0, 0, n1, n2, n3, assign } });

			}
			else if (!m_texels.empty()){
				sscanf(buffer, "%d/%d %d/%d %d/%d", &a, &t1, &b, &t2, &c, &t3);
				face.push_back({ { a, b, c, t1, t2, t3, 0, 0, 0, assign } });

			}
			else {
				sscanf(buffer, "%d %d %d", &a, &b, &c);
				face.push_back({ { a, b, c, 0, 0, 0, 0, 0, 0, assign } });
			}
			break;

		}default: {

			break;
		}
		}

	}



	std::sort(face.begin(), face.end(), compare);

	std::map<int, int> dup;

	for (unsigned int i = 0; i < face.size(); i++){
		dup[face[i][9]]++;
	}



	m_numberOfMeshes = dup.size();
	m_numberOfVertices = m_positions.size();
	m_numberOfTriangles = face.size();

	std::map<int, int>::const_iterator iterDup = dup.begin();

	for (iterDup; iterDup != dup.end(); iterDup++){


		if (name.empty()){

			meshes.push_back(std::shared_ptr<Mesh>(new Mesh(iterDup->second, this)));

		}
		else{

			std::map<std::string, int >::const_iterator iterName = name.begin();
			for (iterName; iterName != name.end(); iterName++){

				if (iterDup->first == iterName->second){

					meshes.push_back(std::shared_ptr<Mesh>(new Mesh("newmtl " + iterName->first, iterDup->second, this)));
				}
			}

		}
	}

	dup.clear();
	name.clear();

	for (int j = 0; j < m_numberOfMeshes; j++){

		if (m_material){

			meshes[j]->m_material = std::shared_ptr<Material>(m_material);

		}
		else if (m_hasMaterials){

			meshes[j]->m_material = std::shared_ptr<Material>(new Phong());

		}

		if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){

			if (meshes[j]->m_material->colorMapPath != ""){

				meshes[j]->m_texture = std::shared_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]));

			}

			if (meshes[j]->m_material->bumpMapPath != ""){
				
				meshes[j]->m_material = static_cast<std::shared_ptr<NormalMap>>(new NormalMap(meshes[j]->m_material));
				static_cast<NormalMap*>(meshes[j]->m_material.get())->setNormalMap(std::unique_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->bumpMapPath)[0])));

			}

		}

		meshes[j]->m_color = Color(1.0f / (j + 1), 1.0f / (j + 1), 1.0f / (j + 1));

	}// end for
	m_materialMesh = meshes[0]->m_material;

	

	if (!m_normals.empty() && !m_texels.empty()) {
		
		m_hasNormals = true;
		m_hasTexels = true;
		for (int i = 0; i < m_numberOfTriangles; i++){

			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[3] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[3] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][2]);

			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[4] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[4] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][2]);

			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[5] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[5] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][2]);
		}
	}else if (!m_normals.empty()){

		m_hasNormals = true;

		for (int i = 0; i < m_numberOfTriangles; i++){

			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][2]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[6] - 1)][2]);

			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][2]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[7] - 1)][2]);

			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][2]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][0]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][1]);
			tmpVertexBuffer.push_back(m_normals[((face[i])[8] - 1)][2]);
		
		}
	}else if (!m_texels.empty()){

		m_hasTexels = true;

		for (int i = 0; i < m_numberOfTriangles; i++){

			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[3] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[3] - 1)][1]);
			
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[4] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[4] - 1)][1]);
			
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][2]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[5] - 1)][0]);
			tmpVertexBuffer.push_back(m_texels[((face[i])[5] - 1)][1]);
		}

	}else{

		for (int i = 0; i < m_numberOfTriangles; i++){

			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[0] - 1)][2]);
			
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[1] - 1)][2]);
			
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][0]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][1]);
			tmpVertexBuffer.push_back(m_positions[((face[i])[2] - 1)][2]);
			
		}

	}

	std::shared_ptr<Triangle> triangle;
	const unsigned int *pTriangle = 0;
	float *pVertex0 = 0;
	float *pVertex1 = 0;
	float *pVertex2 = 0;

	if (m_hasNormals && m_hasTexels){

		indexVBO_PTN(tmpVertexBuffer, m_indexBuffer, m_vertexBuffer);

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

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, cull, smooth));
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

		indexVBO_PN(tmpVertexBuffer, m_indexBuffer, m_vertexBuffer);

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

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, cull, smooth));
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
	
		indexVBO_PT(tmpVertexBuffer, m_indexBuffer, m_vertexBuffer);

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

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, cull, smooth));
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

		indexVBO_P(tmpVertexBuffer, m_indexBuffer, m_vertexBuffer);

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

				triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, cull, smooth));
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


	/*int start = 0;
	int end = meshes[0]->m_numberOfTriangles;

	for (int j = 0; j < m_numberOfMeshes; j++){


		if (j > 0){

			start = end;
			end = end + meshes[j]->m_numberOfTriangles;
		}

		Vector3f a;
		Vector3f b;
		Vector3f c;
		std::shared_ptr<Triangle> triangle;

		for (int i = start; i < end; i++){

			

			a = m_positions[(face[i])[0] - 1];
			b = m_positions[(face[i])[1] - 1];
			c = m_positions[(face[i])[2] - 1];

			m_indexBufferPosition.push_back((face[i])[0] - 1);
			m_indexBufferPosition.push_back((face[i])[1] - 1);
			m_indexBufferPosition.push_back((face[i])[2] - 1);

			meshes[j]->m_indexBuffer.push_back((face[i])[0] - 1);
			meshes[j]->m_indexBuffer.push_back((face[i])[1] - 1);
			meshes[j]->m_indexBuffer.push_back((face[i])[2] - 1);


			meshes[j]->m_xmin = min(a[0] + translate[0], min(b[0] + translate[0], min(c[0] + translate[0], meshes[j]->m_xmin)));
			meshes[j]->m_ymin = min(a[1] + translate[1], min(b[1] + translate[1], min(c[1] + translate[1], meshes[j]->m_ymin)));
			meshes[j]->m_zmin = min(a[2] + translate[2], min(b[2] + translate[2], min(c[2] + translate[2], meshes[j]->m_zmin)));

			meshes[j]->m_xmax = max(a[0] + translate[0], max(b[0] + translate[0], max(c[0] + translate[0], meshes[j]->m_xmax)));
			meshes[j]->m_ymax = max(a[1] + translate[1], max(b[1] + translate[1], max(c[1] + translate[1], meshes[j]->m_ymax)));
			meshes[j]->m_zmax = max(a[2] + translate[2], max(b[2] + translate[2], max(c[2] + translate[2], meshes[j]->m_zmax)));

			triangle = std::shared_ptr<Triangle>(new Triangle(a + translate, b + translate, c + translate, meshes[j]->m_color, cull, smooth));
			triangle->m_texture = meshes[j]->m_texture;
			triangle->m_material = meshes[j]->m_material;


			if (m_texels.size() > 0){

				m_indexBufferTexel.push_back((face[i])[3] - 1);
				m_indexBufferTexel.push_back((face[i])[4] - 1);
				m_indexBufferTexel.push_back((face[i])[5] - 1);

				meshes[j]->m_indexBufferTexel.push_back((face[i])[3] - 1);
				meshes[j]->m_indexBufferTexel.push_back((face[i])[4] - 1);
				meshes[j]->m_indexBufferTexel.push_back((face[i])[5] - 1);

				meshes[j]->m_hasTexels = true;
				m_hasTexels = true;

				triangle->setUV(m_texels[(face[i])[3] - 1], m_texels[(face[i])[4] - 1], m_texels[(face[i])[5] - 1]);
			}


			if (m_normals.size() > 0){

				m_indexBufferNormal.push_back((face[i])[6] - 1);
				m_indexBufferNormal.push_back((face[i])[7] - 1);
				m_indexBufferNormal.push_back((face[i])[8] - 1);

				meshes[j]->m_indexBufferNormal.push_back((face[i])[6] - 1);
				meshes[j]->m_indexBufferNormal.push_back((face[i])[7] - 1);
				meshes[j]->m_indexBufferNormal.push_back((face[i])[8] - 1);

				meshes[j]->m_hasNormals = true;
				m_hasNormals = true;

				triangle->setNormal(m_normals[(face[i])[6] - 1], m_normals[(face[i])[7] - 1], m_normals[(face[i])[8] - 1]);
			}

			meshes[j]->m_triangles.push_back(triangle);
		}

		xmin = min(meshes[j]->m_xmin, xmin);
		ymin = min(meshes[j]->m_ymin, ymin);
		zmin = min(meshes[j]->m_zmin, zmin);

		xmax = max(meshes[j]->m_xmax, xmax);
		ymax = max(meshes[j]->m_ymax, ymax);
		zmax = max(meshes[j]->m_zmax, zmax);
	}*/

	

		std::cout << "Number of faces: " << m_numberOfTriangles << std::endl;
		std::cout << "Number of Meshes: " << m_numberOfMeshes << std::endl;
		calcBounds();


		return true;
	
}

void Model::buildKDTree(){

	std::cout << "Build KDTree!" << std::endl;
	

	for (int j = 0; j < m_numberOfMeshes; j++){



		m_triangles.insert(m_triangles.end(), meshes[j]->m_triangles.begin() , meshes[j]->m_triangles.end() );
		meshes[j]->m_triangles.clear();


	}

	/*for (int i = 0; i < m_triangles.size(); i++){
		std::cout << m_triangles[i]->m_texture << std::endl;
	}*/

	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	std::cout << "Finished KDTree!" << std::endl;

}


void Model::generateNormals(){

	if (m_hasNormals) { return; }
	
	Vector3f normal;

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f normal0;
	Vector3f normal1;
	Vector3f normal2;

	Vector3f edge1;
	Vector3f edge2;



	const unsigned int *pTriangle = 0;

	m_normals = std::vector<Vector3f>(m_numberOfVertices);

	for (int i = 0; i < m_numberOfTriangles; i++){


		pTriangle = &m_indexBufferPosition[i * 3];

		vertex0 = m_positions[pTriangle[0]];
		vertex1 = m_positions[pTriangle[1]];
		vertex2 = m_positions[pTriangle[2]];

		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		normal = Vector3f::cross(edge1, edge2);

		m_normals[pTriangle[0]] = m_normals[pTriangle[0]] + normal;
		m_normals[pTriangle[1]] = m_normals[pTriangle[1]] + normal;
		m_normals[pTriangle[2]] = m_normals[pTriangle[2]] + normal;

	}

	for (int j = 0; j < m_numberOfMeshes; j++){

		for (int i = 0; i < meshes[j]->m_numberOfTriangles; i++){

			pTriangle = &meshes[j]->m_indexBuffer[i * 3];

			Vector3f::normalize(m_normals[pTriangle[0]]);
			Vector3f::normalize(m_normals[pTriangle[1]]);
			Vector3f::normalize(m_normals[pTriangle[2]]);

			meshes[j]->m_triangles[i]->setNormal(m_normals[pTriangle[0]], m_normals[pTriangle[1]], m_normals[pTriangle[2]]);

			//std::cout << m_normals[pTriangle[0]][0] << "  " << m_normals[pTriangle[0]][1] << "  " << m_normals[pTriangle[0]][2] << std::endl;

		}
	}

	m_hasNormals = true;

	m_indexBufferNormal.clear();


	/*for (int j = 0; j < m_numberOfMeshes; j++){
		meshes[j]->generateNormals();
		meshes[j]->m_hasNormals = true;
	}

	m_hasNormals = true;*/
}

void Model::generateNormals2(){

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

		//std::cout << tmpVertex[i + 3 + vertexOffset] << "  " << tmpVertex[i + 4 + vertexOffset] << "  " << tmpVertex[i + 5 + vertexOffset] << std::endl;
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

		m_triangles.clear();
		meshes[j]->m_triangles.clear();
		for (int i = start; i < end; i++){

			pTriangle = &m_indexBuffer[i * 3];

			pVertex0 = &m_vertexBuffer[pTriangle[0] * vertexLength];
			pVertex1 = &m_vertexBuffer[pTriangle[1] * vertexLength];
			pVertex2 = &m_vertexBuffer[pTriangle[2] * vertexLength];


			Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
			Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
			Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

			triangle = std::shared_ptr<Triangle>(new Triangle(a, b, c, meshes[j]->m_color, true, true));
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

void Model::generateTangents(){

	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;

	}

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f edge1;
	Vector3f edge2;

	Vector2f texEdge1;
	Vector2f texEdge2;

	Vector3f normal;
	Vector4f  tangent;
	Vector3f  bitangent;

	float det = 0.0f;
	float nDotT = 0.0f;
	float length = 0.0f;
	float bDotB = 0.0f;

	unsigned int *pTriangle = 0;
	const unsigned int *pTriangleTex = 0;
	const unsigned int *pTriangleNormal = 0;

	std::vector<Vector3f> normals;
	std::vector<Vector4f> tangents;
	std::vector<Vector3f> bitangents;

	tangents = std::vector<Vector4f>(m_numberOfVertices);
	bitangents = std::vector<Vector3f>(m_numberOfVertices);

	if (!m_indexBufferNormal.empty()){

		normals = std::vector<Vector3f>(m_numberOfVertices);
	}
	
	
	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < m_numberOfTriangles; i++){

		pTriangle = &m_indexBufferPosition[i * 3];
		pTriangleTex = &m_indexBufferTexel[i * 3];

		vertex0 = m_positions[pTriangle[0]];
		vertex1 = m_positions[pTriangle[1]];
		vertex2 = m_positions[pTriangle[2]];


		// Calculate the triangle face tangent and bitangent.
		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		texEdge1 = m_texels[pTriangleTex[1]] - m_texels[pTriangleTex[0]];
		texEdge2 = m_texels[pTriangleTex[2]] - m_texels[pTriangleTex[0]];

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
			tangent[3] = 0.0f;

			

			bitangent = ((edge2 * texEdge1[0]) - (edge1 * texEdge2[0])) * det;

			/*bitangent[0] = ((edge2[0] * texEdge1[0]) - (edge1[0] * texEdge2[0])) * det;
			bitangent[1] = ((edge2[1] * texEdge1[0]) - (edge1[1] * texEdge2[0])) * det;
			bitangent[2] = ((edge2[2] * texEdge1[0]) - (edge1[2] * texEdge2[0])) * det;*/
		}

		//std::cout << bitangent[0] << "  " << bitangent[1] << "  " << bitangent[2] << std::endl;
		// Accumulate the tangents and bitangents.
		tangents[pTriangle[0]] = tangents[pTriangle[0]] + tangent;
		tangents[pTriangle[1]] = tangents[pTriangle[1]] + tangent;
		tangents[pTriangle[2]] = tangents[pTriangle[2]] + tangent;

		bitangents[pTriangle[0]] = bitangents[pTriangle[0]] + bitangent;
		bitangents[pTriangle[1]] = bitangents[pTriangle[1]] + bitangent;
		bitangents[pTriangle[2]] = bitangents[pTriangle[2]] + bitangent;

	
		
		// Order the normals
		if (!m_indexBufferNormal.empty()){

			pTriangleNormal = &m_indexBufferNormal[i * 3];

			if (normals[pTriangle[0]].null()){

				normals[pTriangle[0]] = m_normals[pTriangleNormal[0]];
			}

			if (normals[pTriangle[1]].null()){

				normals[pTriangle[1]] = m_normals[pTriangleNormal[1]];
			}

			if (normals[pTriangle[2]].null()){

				normals[pTriangle[2]] = m_normals[pTriangleNormal[2]];
			}
		}

		

	}

	if (!m_indexBufferNormal.empty()){
		
		m_normals.clear();
		m_normals.swap(normals);
	}
	
	// Orthogonalize and normalize the vertex tangents.
	for (int i = 0; i < m_numberOfVertices; i++){

		// Gram-Schmidt orthogonalize tangent with normal.
		nDotT = m_normals[i][0] * tangents[i][0] +
			m_normals[i][1] * tangents[i][1] +
			m_normals[i][2] * tangents[i][2];

		tangents[i][0] -= m_normals[i][0] * nDotT;
		tangents[i][1] -= m_normals[i][1] * nDotT;
		tangents[i][2] -= m_normals[i][2] * nDotT;

		

		// Normalize the tangent.
		Vector4f::normalize(tangents[i]);

		

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

		//std::cout << tangents[i][0] << "  " << tangents[i][1] << "  " << tangents[i][2] << "  " << tangents[i][3] << std::endl;
	}



	for (int j = 0; j < m_numberOfMeshes; j++){

		for (int i = 0; i < meshes[j]->m_numberOfTriangles; i++){

			pTriangle = &meshes[j]->m_indexBuffer[i * 3];

			meshes[j]->m_triangles[i]->setTangents(tangents[pTriangle[0]], tangents[pTriangle[1]], tangents[pTriangle[2]]);
			meshes[j]->m_triangles[i]->setBiTangents(bitangents[pTriangle[0]], bitangents[pTriangle[1]], bitangents[pTriangle[2]]);
		}
	}
	
	m_hasNormals = true;
	m_hasTangents = true;

	normals.clear();
	tangents.clear();
	bitangents.clear();

	m_positions.clear();
	m_normals.clear();
	m_texels.clear();
	m_indexBufferPosition.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();
	

	/*for (int j = 0; j < m_numberOfMeshes; j++){
		meshes[j]->generateTangents();
	}*/

}

void Model::generateTangents2(){
	
	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals2();
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


		// Calculate the handedness of the local tangent space.
		// The bitangent vector is the cross product between the triangle face
		// normal vector and the calculated tangent vector. The resulting
		// bitangent vector should be the same as the bitangent vector
		// calculated from the set of linear equations above. If they point in
		// different directions then we need to invert the cross product
		// calculated bitangent vector. We store this scalar multiplier in the
		// tangent vector's 'w' component so that the correct bitangent vector
		// can be generated in the normal mapping shader's vertex shader.
		//
		// Normal maps have a left handed coordinate system with the origin
		// located at the top left of the normal map texture. The x coordinates
		// run horizontally from left to right. The y coordinates run
		// vertically from top to bottom. The z coordinates run out of the
		// normal map texture towards the viewer. Our handedness calculations
		// must take this fact into account as well so that the normal mapping
		// shader's vertex shader will generate the correct bitangent vectors.
		// Some normal map authoring tools such as Crazybump
		// (http://www.crazybump.com/) includes options to allow you to control
		// the orientation of the normal map normal's y-axis.

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

		m_triangles.clear();
		meshes[j]->m_triangles.clear();
		for (int i = start; i < end; i++){
		
			pTriangle = &m_indexBuffer[i * 3];

			pVertex0 = &m_vertexBuffer[pTriangle[0] * 15];
			pVertex1 = &m_vertexBuffer[pTriangle[1] * 15];
			pVertex2 = &m_vertexBuffer[pTriangle[2] * 15];

			


			Vector3f a = Vector3f(pVertex0[0], pVertex0[1], pVertex0[2]);
			Vector3f b = Vector3f(pVertex1[0], pVertex1[1], pVertex1[2]);
			Vector3f c = Vector3f(pVertex2[0], pVertex2[1], pVertex2[2]);

			triangle = std::shared_ptr<Triangle>(new Triangle(a , b , c , meshes[j]->m_color, true, true));
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

	m_positions.clear();
	m_normals.clear();
	m_texels.clear();
	m_indexBufferPosition.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();

	m_hasTangents = true;
}

////////////////////////////////////////////////////Mesh////////////////////////////////////

Mesh::Mesh(std::string mltName, int numberTriangles, Model *model){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberOfTriangles = numberTriangles;
	m_mltName = mltName;
	m_texture = NULL;
	//m_normalMap = NULL;
	m_material = NULL;
	m_model = std::unique_ptr<Model>(model);

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

Mesh::Mesh(int numberTriangles, Model *model){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberOfTriangles = numberTriangles;
	m_texture = NULL;
	//m_normalMap = NULL;
	m_material = NULL;
	m_model = std::unique_ptr<Model>(model);

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

Mesh::~Mesh(){

}


void Mesh::generateNormals(){

	if (m_hasNormals) { return; }
	
	Vector3f normal;

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f normal0;
	Vector3f normal1;
	Vector3f normal2;

	Vector3f edge1;
	Vector3f edge2;

	

	const unsigned int *pTriangle = 0;
	
	for (int i = 0; i < m_model->m_numberOfVertices; i++){

		m_normals.push_back(Vector3f(0.0, 0.0, 0.0));
		
	}

	for (int i = 0; i < m_numberOfTriangles; i++){

		
		pTriangle = &m_indexBuffer[i * 3];

		vertex0 = m_model->m_positions[pTriangle[0]];
		vertex1 = m_model->m_positions[pTriangle[1]];
		vertex2 = m_model->m_positions[pTriangle[2]];

		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		normal = Vector3f::cross(edge1, edge2);

		m_normals[pTriangle[0]] = m_normals[pTriangle[0]] + normal;
		m_normals[pTriangle[1]] = m_normals[pTriangle[1]] + normal;
		m_normals[pTriangle[2]] = m_normals[pTriangle[2]] + normal;

	}

	
	for (int i = 0; i < m_numberOfTriangles; i++){
	
		pTriangle = &m_indexBuffer[i * 3];

		Vector3f::normalize(m_normals[pTriangle[0]]);
		Vector3f::normalize(m_normals[pTriangle[1]]);
		Vector3f::normalize(m_normals[pTriangle[2]]);

		m_triangles[i]->setNormal(m_normals[pTriangle[0]], m_normals[pTriangle[1]], m_normals[pTriangle[2]]);
		
	}

	m_indexBufferNormal.clear();
	m_hasNormals = true;

}

void Mesh::generateTangents(){

	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated!" << std::endl;

	}

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f edge1;
	Vector3f edge2;

	Vector2f texEdge1;
	Vector2f texEdge2;

	Vector3f normal;
	Vector4f  tangent;
	Vector3f  bitangent;

	float det = 0.0f;
	float nDotT = 0.0f;
	float length = 0.0f;
	float bDotB = 0.0f;

	unsigned int *pTriangle = 0;
	const unsigned int *pTriangleTex = 0;
	const unsigned int *pTriangleNormal = 0;

	std::vector<Vector3f> tmpNormals;

	for (int i = 0; i < m_model->m_numberOfVertices; i++){

		m_tangents.push_back(Vector4f(0.0, 0.0, 0.0, 0.0));
		

		m_bitangents.push_back(Vector3f(0.0, 0.0, 0.0));
		

		if (!m_indexBufferNormal.empty()){

			tmpNormals.push_back(Vector3f(0.0, 0.0, 0.0));

		}
	}

	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < m_model->m_numberOfTriangles; i++){

		pTriangle = &m_indexBuffer[i * 3];
		pTriangleTex = &m_indexBufferTexel[i * 3];
	
		vertex0 = m_model->m_positions[pTriangle[0]];
		vertex1 = m_model->m_positions[pTriangle[1]];
		vertex2 = m_model->m_positions[pTriangle[2]];


		// Calculate the triangle face tangent and bitangent.
		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		texEdge1 = m_model->m_texels[pTriangleTex[1]] - m_model->m_texels[pTriangleTex[0]];
		texEdge2 = m_model->m_texels[pTriangleTex[2]] - m_model->m_texels[pTriangleTex[0]];

		det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

		if (fabs(det) < 1e-6f){

			tangent[0] = 1.0f;
			tangent[1] = 0.0f;
			tangent[2] = 0.0f;

			bitangent[0] = 0.0f;
			bitangent[1] = 1.0f;
			bitangent[2] = 0.0f;

		}else{

			det = 1.0f/ det;

		
			tangent[0] = ((texEdge2[1] * edge1[0]) - (texEdge1[1] * edge2[0])) * det;
			tangent[1] = ((texEdge2[1] * edge1[1]) - (texEdge1[1] * edge2[1])) * det;
			tangent[2] = ((texEdge2[1] * edge1[2]) - (texEdge1[1] * edge2[2])) * det;
			tangent[3] = 0.0f;

			bitangent = ((edge2 * texEdge1[0]) - (edge1 * texEdge2[0])) * det;

			/*bitangent[0] = ((edge2[0] * texEdge1[0]) - (edge1[0] * texEdge2[0])) * det;
			bitangent[1] = ((edge2[1] * texEdge1[0]) - (edge1[1] * texEdge2[0])) * det;
			bitangent[2] = ((edge2[2] * texEdge1[0]) - (edge1[2] * texEdge2[0])) * det;*/
			

		}

		// Accumulate the tangents and bitangents.
		m_tangents[pTriangle[0]] = m_tangents[pTriangle[0]] + tangent;
		m_tangents[pTriangle[1]] = m_tangents[pTriangle[1]] + tangent;
		m_tangents[pTriangle[2]] = m_tangents[pTriangle[2]] + tangent;

		m_bitangents[pTriangle[0]] = m_bitangents[pTriangle[0]] + bitangent;
		m_bitangents[pTriangle[1]] = m_bitangents[pTriangle[1]] + bitangent;
		m_bitangents[pTriangle[2]] = m_bitangents[pTriangle[2]] + bitangent;

		
		// Order the normals
		if (!m_model->m_indexBufferNormal.empty()){

			pTriangleNormal = &m_indexBufferNormal[i * 3];

		

			if (tmpNormals[pTriangle[0]].null()){

				tmpNormals[pTriangle[0]] = m_model->m_normals[pTriangleNormal[0]];
			}

			if (tmpNormals[pTriangle[1]].null()){

				tmpNormals[pTriangle[1]] = m_model->m_normals[pTriangleNormal[1]];
			}

			if (tmpNormals[pTriangle[2]].null()){

				tmpNormals[pTriangle[2]] = m_model->m_normals[pTriangleNormal[2]];
			}
		}

	}


	if (!m_model->m_indexBufferNormal.empty()){

		m_normals.clear();
		m_normals.swap(tmpNormals);
	}
	// Orthogonalize and normalize the vertex tangents.
	for (int i = 0; i < m_model->m_numberOfVertices; i++){

		
		// Gram-Schmidt orthogonalize tangent with normal.

		nDotT = m_normals[i][0] * m_tangents[i][0] +
			m_normals[i][1] * m_tangents[i][1] +
			m_normals[i][2] * m_tangents[i][2];

		m_tangents[i][0] -= m_normals[i][0] * nDotT;
		m_tangents[i][1] -= m_normals[i][1] * nDotT;
		m_tangents[i][2] -= m_normals[i][2] * nDotT;

		// Normalize the tangent.
		Vector4f::normalize(m_tangents[i]);
		
		
		
		// Calculate the handedness of the local tangent space.
		// The bitangent vector is the cross product between the triangle face
		// normal vector and the calculated tangent vector. The resulting
		// bitangent vector should be the same as the bitangent vector
		// calculated from the set of linear equations above. If they point in
		// different directions then we need to invert the cross product
		// calculated bitangent vector. We store this scalar multiplier in the
		// tangent vector's 'w' component so that the correct bitangent vector
		// can be generated in the normal mapping shader's vertex shader.
		//
		// Normal maps have a left handed coordinate system with the origin
		// located at the top left of the normal map texture. The x coordinates
		// run horizontally from left to right. The y coordinates run
		// vertically from top to bottom. The z coordinates run out of the
		// normal map texture towards the viewer. Our handedness calculations
		// must take this fact into account as well so that the normal mapping
		// shader's vertex shader will generate the correct bitangent vectors.
		// Some normal map authoring tools such as Crazybump
		// (http://www.crazybump.com/) includes options to allow you to control
		// the orientation of the normal map normal's y-axis.

		bitangent[0] = (m_normals[i][1] * m_tangents[i][2]) -
			(m_normals[i][2] * m_tangents[i][1]);
		bitangent[1] = (m_normals[i][2] * m_tangents[i][0]) -
			(m_normals[i][0] * m_tangents[i][2]);
		bitangent[2] = (m_normals[i][0] * m_tangents[i][1]) -
			(m_normals[i][1] * m_tangents[i][0]);

		bDotB = bitangent[0] * m_bitangents[i][0] +
			bitangent[1] * m_bitangents[i][1] +
			bitangent[2] * m_bitangents[i][2];

		// Calculate handedness
		m_tangents[i][3] = (bDotB < 0.0f) ? 1.0f : -1.0f;
		m_bitangents[i] = bitangent;
	
	}

	for (int i = 0; i <m_numberOfTriangles; i++){

		pTriangle = &m_model->m_indexBufferPosition[i * 3];

		m_triangles[i]->setTangents(m_tangents[pTriangle[0]], m_tangents[pTriangle[1]], m_tangents[pTriangle[2]]);
		m_triangles[i]->setBiTangents(m_bitangents[pTriangle[0]], m_bitangents[pTriangle[1]], m_bitangents[pTriangle[2]]);
	}

	m_hasTangents = true;
}



bool Mesh::readMaterial(const char* filename){
	
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

		}
		else if ((*lines[i])[0] == 'N' && (*lines[i])[1] == 's'){
			
			int tmp;
			sscanf(lines[i]->c_str(), "Ns %i", &tmp);
			//static_cast<std::shared_ptr<Phong>>(m_material.get())->m_shinies = tmp;
			m_material->m_shinies = tmp;
			

		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'a'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ka %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_ambient = Color(tmpx, tmpy, tmpz);
		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'd'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Kd %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_diffuse =  Color(tmpx, tmpy, tmpz);

		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 's'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ks %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_specular =  Color(tmpx, tmpy, tmpz);

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

// Returns true iif v1 can be considered equal to v2
bool is_near(float v1, float v2){
	return v1 == v2;
}

// Searches through all already-exported vertices
// for a similar one.
// Similar = same position + same UVs + same normal
bool getSimilarVertexIndex(float p1, float p2, float p3, float n1, float n2, float n3, std::vector<float>& out_vertices, int &index)

{
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i = i + 6){
		if (
			is_near(p1, out_vertices[i]) &&
			is_near(p2, out_vertices[i + 1]) &&
			is_near(p3, out_vertices[i + 2]) &&
			is_near(n1, out_vertices[i + 3]) &&
			is_near(n2, out_vertices[i + 4]) &&
			is_near(n3, out_vertices[i + 5])
			){
			index = i / 6;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

bool getSimilarVertexIndex(float p1, float p2, float p3, float t1, float t2, float n1, float n2, float n3, std::vector<float>& out_vertices, int &index)

{
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i = i + 8){
		if (
			is_near(p1, out_vertices[i]) &&
			is_near(p2, out_vertices[i + 1]) &&
			is_near(p3, out_vertices[i + 2]) &&
			is_near(t1, out_vertices[i + 3]) &&
			is_near(t2, out_vertices[i + 4]) &&
			is_near(n1, out_vertices[i + 5]) &&
			is_near(n2, out_vertices[i + 6]) &&
			is_near(n3, out_vertices[i + 7])
			){
			index = i / 8;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

bool getSimilarVertexIndex(float p1, float p2, float p3, float t1, float t2, std::vector<float>& out_vertices, int &index)

{
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i = i + 5){
		if (
			is_near(p1, out_vertices[i]) &&
			is_near(p2, out_vertices[i + 1]) &&
			is_near(p3, out_vertices[i + 2]) &&
			is_near(t1, out_vertices[i + 3]) &&
			is_near(t2, out_vertices[i + 4])
			){
			index = i / 5;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}


bool getSimilarVertexIndex(float p1, float p2, float p3, std::vector<float>& out_vertices, int &index){
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i = i + 3){
		if (
			is_near(p1, out_vertices[i]) &&
			is_near(p2, out_vertices[i + 1]) &&
			is_near(p3, out_vertices[i + 2])

			){
			index = i / 3;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}


void Model::indexVBO_P(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices){
	// For each input vertex
	for (int i = 0; i<in_vertices.size(); i = i + 3){

		// Try to find a similar vertex in out_XXXX
		int index;
		bool found = getSimilarVertexIndex(in_vertices[i],
			in_vertices[i + 1],
			in_vertices[i + 2],
			out_vertices, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !



			out_indices.push_back(index);
		}
		else{ // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_vertices.push_back(in_vertices[i + 1]);
			out_vertices.push_back(in_vertices[i + 2]);
			out_indices.push_back((int)out_vertices.size() / 3 - 1);
		}
	}
}

void Model::indexVBO_PN(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices){
	// For each input vertex
	for (int i = 0; i<in_vertices.size(); i = i + 6){

		// Try to find a similar vertex in out_XXXX
		int index;
		bool found = getSimilarVertexIndex(in_vertices[i],
			in_vertices[i + 1],
			in_vertices[i + 2],
			in_vertices[i + 3],
			in_vertices[i + 4],
			in_vertices[i + 5],
			out_vertices, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !

			out_indices.push_back(index);
		}
		else{ // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_vertices.push_back(in_vertices[i + 1]);
			out_vertices.push_back(in_vertices[i + 2]);
			out_vertices.push_back(in_vertices[i + 3]);
			out_vertices.push_back(in_vertices[i + 4]);
			out_vertices.push_back(in_vertices[i + 5]);

			out_indices.push_back((int)out_vertices.size() / 6 - 1);
		}
	}
}



void Model::indexVBO_PT(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices){
	// For each input vertex
	for (int i = 0; i<in_vertices.size(); i = i + 5){

		// Try to find a similar vertex in out_XXXX
		int index;
		bool found = getSimilarVertexIndex(in_vertices[i],
			in_vertices[i + 1],
			in_vertices[i + 2],
			in_vertices[i + 3],
			in_vertices[i + 4],
			out_vertices, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !

			out_indices.push_back(index);
		}
		else{ // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_vertices.push_back(in_vertices[i + 1]);
			out_vertices.push_back(in_vertices[i + 2]);
			out_vertices.push_back(in_vertices[i + 3]);
			out_vertices.push_back(in_vertices[i + 4]);

			out_indices.push_back((int)out_vertices.size() / 5 - 1);
		}
	}
}


void Model::indexVBO_PTN(std::vector<float> & in_vertices, std::vector<unsigned int> & out_indices, std::vector<float> & out_vertices){
	// For each input vertex
	for (int i = 0; i<in_vertices.size(); i = i + 8){

		// Try to find a similar vertex in out_XXXX
		int index;
		bool found = getSimilarVertexIndex(in_vertices[i],
			in_vertices[i + 1],
			in_vertices[i + 2],
			in_vertices[i + 3],
			in_vertices[i + 4],
			in_vertices[i + 5],
			in_vertices[i + 6],
			in_vertices[i + 7],
			out_vertices, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !

			out_indices.push_back(index);
		}
		else{ // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_vertices.push_back(in_vertices[i + 1]);
			out_vertices.push_back(in_vertices[i + 2]);
			out_vertices.push_back(in_vertices[i + 3]);
			out_vertices.push_back(in_vertices[i + 4]);
			out_vertices.push_back(in_vertices[i + 5]);
			out_vertices.push_back(in_vertices[i + 6]);
			out_vertices.push_back(in_vertices[i + 7]);

			out_indices.push_back((int)out_vertices.size() / 8 - 1);
		}
	}
}
