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

		return (invT *  m_KDTree->m_primitive->getNormal(a_pos)).normalize();
	}
	
	return Vector3f(0.0, 0.0, 0.0);
	
}

std::shared_ptr<Material> Model::getMaterial(){

	if (m_material){

		return m_material;

	}else{
		
		return m_KDTree->m_primitive->m_material;
	}
}

bool Model::loadObject(const char* filename, bool cull, bool smooth){

	return loadObject(filename, Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 1.0, cull, smooth);
}

bool compare(const std::array<int, 10> &i_lhs, const std::array<int, 10> &i_rhs){

	return i_lhs[9] < i_rhs[9];
}

bool Model::loadObject(const char* a_filename, Vector3f &axis, float degree, Vector3f &translate, float scale, bool cull, bool smooth){

		std::string filename(a_filename);

		const size_t index = filename.rfind('/');

		if (std::string::npos != index){

			m_modelDirectory = filename.substr(0, index);
		}


		std::vector<std::array<int, 10>> face;
		std::map<std::string, int> name;


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
							  m_positions.push_back(Vector3f(position[0] * scale, position[1] * scale, position[2] * scale));

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

			}else if (m_hasMaterials){

				meshes[j]->m_material = std::shared_ptr<Material>(new Phong());
				
			}

			if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){

				if (meshes[j]->m_material->colorMapPath != ""){

					meshes[j]->m_texture = std::shared_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]));
					
				}

			}

			meshes[j]->m_color = Color(1.0f / (j + 1), 1.0f / (j + 1), 1.0f / (j + 1));

		}// end for


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

				m_indexBuffer.push_back((face[i])[0] - 1);
				m_indexBuffer.push_back((face[i])[1] - 1);
				m_indexBuffer.push_back((face[i])[2] - 1);
				
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


		}



		std::cout << "Number of faces: " << face.size() << std::endl;
		calcBounds();


		return true;
	
}

void Model::buildKDTree(){

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


		pTriangle = &m_indexBuffer[i * 3];

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



////////////////////////////////////////////////////Mesh////////////////////////////////////

Mesh::Mesh(std::string mltName, int numberTriangles, Model *model){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberOfTriangles = numberTriangles;
	m_mltName = mltName;
	m_texture = NULL;
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

				
			}
			else if (strstr(identifierBuffer, "map_bump") != 0){

				m_material->bumpMapPath = valueBuffer;

			}

		}
	}



	for (unsigned int i = 0; i < lines.size(); i++){

		delete lines[i];
	}

	return true;
}
