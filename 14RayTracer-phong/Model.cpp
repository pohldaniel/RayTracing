#include "Model.h"
#include "KDTree.h"

Model::Model() :OrientablePrimitive() {

	m_hasMaterial = false;
	m_hasnormal = false;
	m_texture = NULL;
	m_color = Color(1.0, 1.0, 0.0);

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}

Model::Model(Color color) :OrientablePrimitive(color) {

	m_hasMaterial = false;
	m_hasnormal = false;
	m_texture = NULL;
	m_color = color;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}



Model::~Model(){


}


void Model::hit(const Ray& a_Ray, Hit &hit){

	// find the nearest intersection
	m_KDTree->intersectRec(a_Ray, hit);

}

bool Model::loadObject(const char* filename){

	return loadObject(filename, Vector3f(1.0, 0.0, 0.0), 1.0, Vector3f(0.0, 0.0, 0.0), 1.0);
}

void Model::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	Model::bounds = true;
}

Color Model::getColor(Vector3f& a_Pos){

	if (m_KDTree->m_primitive->m_mesh->m_texture){

		m_KDTree->m_primitive->setTexture(m_KDTree->m_primitive->m_mesh->m_texture);

		return m_KDTree->m_primitive->getColor(a_Pos);

	}
	else if (m_texture){

		m_KDTree->m_primitive->setTexture(m_texture);

		return m_KDTree->m_primitive->getColor(a_Pos);

	}else {

		return m_color;
	}
}

Vector3f  Model::getNormal(Vector3f& a_Pos){

	if (m_hasnormal){

		Vector3f normal;

		normal = m_KDTree->m_primitive->getNormal(a_Pos);


		//calculate the transpose of invT with the normal
		return (invT * normal).normalize();

		//return (invT.transpose() * Vector4f(normal, 0.0)).normalize();
		return (T * Vector4f(normal, 0.0)).normalize();
	}
	else{

		return Vector3f(1.0, 0.0, 0.0);
	}
}

Material* Model::getMaterial(){

	return m_KDTree->m_primitive->m_mesh->m_material;
}

bool compare(const std::array<int, 10> &i_lhs, const std::array<int, 10> &i_rhs){

	return i_lhs[9] < i_rhs[9];
}

bool Model::loadObject(const char* a_filename, Vector3f &axis, float degree, Vector3f &translate, float scale){

	std::string filename(a_filename);

	const size_t index = filename.rfind('/');

	if (std::string::npos != index){

		m_modelDirectory = filename.substr(0, index);
	}

	std::vector<std::string*>coord;
	std::vector<std::array<int, 10>> face;

	std::vector<Vector3f*> positionCoords;
	std::vector<Vector3f*> normalCoords;
	std::vector<Vector2f*> textureCoords;

	std::ifstream in(filename);

	std::map<std::string, int> name;


	int countMesh = 0;
	int assign = 0;
	char buffer[250];


	if (!in.is_open()){

		std::cout << "File not found" << std::endl;
		return false;
	}

	std::string line;
	while (getline(in, line)){
		coord.push_back(new std::string(line));

	}
	in.close();

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degree);

	for (int i = 0; i < coord.size(); i++){
		
		if ((*coord[i])[0] == '#'){

			continue;

		}
		else if ((*coord[i])[0] == 'm'){

			sscanf(coord[i]->c_str(), "%s %s", buffer, buffer);
			m_mltPath = buffer;

			m_hasMaterial = true;

		}else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == ' '){


			float tmpx, tmpy, tmpz;
			sscanf(coord[i]->c_str(), "v %f %f %f", &tmpx, &tmpy, &tmpz);

			Vector3f position = Vector3f(tmpx, tmpy, tmpz);

			position = rotMtx * position;

			positionCoords.push_back(new Vector3f(position[0] * scale, position[1] * scale, position[2] * scale));
		}
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == 't'){

			float tmpu, tmpv;
			sscanf(coord[i]->c_str(), "vt %f %f", &tmpu, &tmpv);
			textureCoords.push_back(new Vector2f(tmpu, tmpv));
		}
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == 'n'){
			float tmpx, tmpy, tmpz;

			sscanf(coord[i]->c_str(), "vn %f %f %f", &tmpx, &tmpy, &tmpz);

			Vector3f normal = Vector3f(tmpx, tmpy, tmpz);
			Matrix4f rotMtx;
			rotMtx.rotate(axis, degree);
			normal = rotMtx * normal;
			normalCoords.push_back(new Vector3f(normal[0], normal[1], normal[2]));

		}else if ((*coord[i])[0] == 'u' &&  m_hasMaterial){

			sscanf(coord[i]->c_str(), "%s %s", buffer, buffer);



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
			//std::cout << assign << "  " << buffer << std::endl;


		}else if ((*coord[i])[0] == 'g' && !m_hasMaterial){
		
			sscanf(coord[i]->c_str(), "%s %s", buffer, buffer);

			countMesh++;
			assign = countMesh;
			name[buffer] = countMesh;

		}else if ((*coord[i])[0] == 'f'){

			int a, b, c, n1, n2, n3, t1, t2, t3;


			if (!textureCoords.empty() && !normalCoords.empty()){
				sscanf(coord[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d ", &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);

				face.push_back({ { a, b, c, t1, t2, t3, n1, n2, n3, assign } });

			}else if (!normalCoords.empty()){

				sscanf(coord[i]->c_str(), "f %d//%d %d//%d %d//%d", &a, &n1, &b, &n2, &c, &n3);

				face.push_back({ { a, b, c, 0, 0, 0, n1, n2, n3, assign } });


			}else if (!textureCoords.empty()){

				sscanf(coord[i]->c_str(), "f %d/%d %d/%d %d/%d", &a, &t1, &b, &t2, &c, &t3);

				face.push_back({ { a, b, c, t1, t2, t3, 0, 0, 0, assign } });


			}else {

				sscanf(coord[i]->c_str(), "f %d %d %d", &a, &b, &c);

				face.push_back({ { a, b, c, 0, 0, 0, 0, 0, 0, assign } });
			}


		}
	}

	std::sort(face.begin(), face.end(), compare);

	std::map<int, int> dup;

	for (int i = 0; i < face.size(); i++){
		dup[face[i][9]]++;
	}

	

	m_numberOfMeshes = dup.size();

	std::map<int, int>::const_iterator iterDup = dup.begin();

	for (iterDup; iterDup != dup.end(); iterDup++){


		if (name.empty()){

			meshes.push_back(new Mesh(iterDup->second));

		}
		else{

			std::map<std::string, int >::const_iterator iterName = name.begin();
			for (iterName; iterName != name.end(); iterName++){

				if (iterDup->first == iterName->second){



					meshes.push_back(new Mesh("newmtl " + iterName->first, iterDup->second));
				}
			}

		}
	}

	dup.clear();
	name.clear();

	int start = 0;
	int end = meshes[0]->m_numberTriangles;

	

	for (int j = 0; j < m_numberOfMeshes; j++){

		
		if (j > 0){

			start = end;
			end = end + meshes[j]->m_numberTriangles;
		}

		Vector3f *normal;
		Vector3f *a;
		Vector3f *b;
		Vector3f *c;
		Triangle *triangle;


		for (int i = start; i < end; i++){

			a = positionCoords[(face[i])[0] - 1];
			b = positionCoords[(face[i])[1] - 1];
			c = positionCoords[(face[i])[2] - 1];

			meshes[j]->m_xmin = min(a->getVec()[0] + translate.getVec()[0], min(b->getVec()[0] + translate.getVec()[0], min(c->getVec()[0] + translate.getVec()[0], meshes[j]->m_xmin)));
			meshes[j]->m_ymin = min(a->getVec()[1] + translate.getVec()[1], min(b->getVec()[1] + translate.getVec()[1], min(c->getVec()[1] + translate.getVec()[1], meshes[j]->m_ymin)));
			meshes[j]->m_zmin = min(a->getVec()[2] + translate.getVec()[2], min(b->getVec()[2] + translate.getVec()[2], min(c->getVec()[2] + translate.getVec()[2], meshes[j]->m_zmin)));

			meshes[j]->m_xmax = max(a->getVec()[0] + translate.getVec()[0], max(b->getVec()[0] + translate.getVec()[0], max(c->getVec()[0] + translate.getVec()[0], meshes[j]->m_xmax)));
			meshes[j]->m_ymax = max(a->getVec()[1] + translate.getVec()[1], max(b->getVec()[1] + translate.getVec()[1], max(c->getVec()[1] + translate.getVec()[1], meshes[j]->m_ymax)));
			meshes[j]->m_zmax = max(a->getVec()[2] + translate.getVec()[2], max(b->getVec()[2] + translate.getVec()[2], max(c->getVec()[2] + translate.getVec()[2], meshes[j]->m_zmax)));

			triangle = new Triangle(*a + translate, *b + translate, *c + translate, Color(1.0, 0.0, 1.0));
			triangle->m_mesh = meshes[j];

			if (textureCoords.size() > 0){

				triangle->setUV(textureCoords[(face[i])[3] - 1]->getVec()[0],
					textureCoords[(face[i])[4] - 1]->getVec()[0],
					textureCoords[(face[i])[5] - 1]->getVec()[0],
					textureCoords[(face[i])[3] - 1]->getVec()[1],
					textureCoords[(face[i])[4] - 1]->getVec()[1],
					textureCoords[(face[i])[5] - 1]->getVec()[1]);

			}


			if (normalCoords.size() > 0){

				m_hasnormal = true;
				triangle->setNormal(*normalCoords[(face[i])[6] - 1], *normalCoords[(face[i])[7] - 1], *normalCoords[(face[i])[8] - 1]);
			}

			triangles.push_back(triangle);
		}


		meshes[j]->m_material = new Material();
		if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){

			meshes[j]->m_texture = new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]);

		}else{
			

			meshes[j]->m_color = Color(1.0 / (j + 1), 1.0 / (j + 1), 1.0 / (j + 1));
			meshes[j]->m_material->m_ambient2 = new Color(0.1, 0.1, 0.1);
			meshes[j]->m_material->m_diffuse2 = new Color(0.7, 0.7, 0.7);
			meshes[j]->m_material->m_specular2 = new Color(0.4, 0.4, 0.4);
			meshes[j]->m_material->m_shinies = 20;

		
		}

		
		xmin = min(meshes[j]->m_xmin, xmin);
		ymin = min(meshes[j]->m_ymin, ymin);
		zmin = min(meshes[j]->m_zmin, zmin);

		xmax = max(meshes[j]->m_xmax, xmax);
		ymax = max(meshes[j]->m_ymax, ymax);
		zmax = max(meshes[j]->m_zmax, zmax);
		
	}


	
	std::cout << "Number of faces: " << triangles.size() << std::endl;
	calcBounds();
	std::cout << "Build KDTree!" << std::endl;

	m_KDTree = new KDTree();
	m_KDTree->buildTree(triangles, box);

	std::cout << "Finished KDTree!" << std::endl;

	for (int i = 0; i < coord.size(); i++){
		delete coord[i];

	}

	for (int i = 0; i < normalCoords.size(); i++){
		delete normalCoords[i];
	}
	for (int i = 0; i < positionCoords.size(); i++){
		delete positionCoords[i];
	}

	return true;
}

////////////////////////////////////////////////////Mesh////////////////////////////////////

Mesh::Mesh(std::string mltName, int numberTriangles){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberTriangles = numberTriangles;
	m_mltName = mltName;
	m_texture = NULL;
	m_material = NULL;

	m_hasNormals = false;
	m_hasTextureCoords = false;
	m_hasTangents = false;

	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

Mesh::Mesh(int numberTriangles){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberTriangles = numberTriangles;
	m_texture = NULL;
	m_material = NULL;

	m_hasNormals = false;
	m_hasTextureCoords = false;
	m_hasTangents = false;


	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

Mesh::~Mesh(){


}


bool Mesh::mltCompare(std::string* mltName){
	return *mltName == m_mltName;
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


	for (int i = 0; i < lines.size(); i++){

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
			
			float tmp;
			sscanf(lines[i]->c_str(), "Ns %f", &tmp);

			m_material->m_shinies = tmp;
			

		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'a'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ka %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_ambient2 = new Color(tmpx, tmpy, tmpz);
		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'd'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Kd %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_diffuse2 = new Color(tmpx, tmpy, tmpz);

		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 's'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ks %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_specular2 = new Color(tmpx, tmpy, tmpz);

		}else if ((*lines[i])[0] == 'm'){


			char identifierBuffer[20], valueBuffer[250];;
			sscanf(lines[i]->c_str(), "%s %s", identifierBuffer, valueBuffer);

			if (strstr(identifierBuffer, "map_Kd") != 0){

				m_material->colorMapPath = valueBuffer;

				
			}
			else if (strstr(identifierBuffer, "map_bump") != 0){

				m_material->bumpMapPath = valueBuffer;

			}

		}
	}



	for (int i = 0; i < lines.size(); i++){

		delete lines[i];
	}
}
