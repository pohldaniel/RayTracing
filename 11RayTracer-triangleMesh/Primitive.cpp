#include "Primitive.h"

Primitive::Primitive(){
	orientable = false;
	color = Color(1.0, 0.0, 0.0);
	Primitive::invT.identity();
}

Primitive::Primitive(const Color& color){
	orientable = false;
	Primitive::color = color;
	Primitive::invT.identity();
}

Primitive::~Primitive(){

}

Color Primitive::getColor(){

	return color;
}

//////////////////////////////////////////////////////////////////////////////////////////


OrientablePrimitive::OrientablePrimitive() :Primitive(){
	orientable = true;

}


OrientablePrimitive::OrientablePrimitive(const Color& color) : Primitive(color){
	orientable = true;

}

OrientablePrimitive::~OrientablePrimitive(){


}

void OrientablePrimitive::rotate(const Vector3f &axis, float degrees){

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degrees);


	Matrix4f invRotMtx = Matrix4f(rotMtx[0][0], rotMtx[1][0], rotMtx[2][0], rotMtx[3][0],
		rotMtx[0][1], rotMtx[1][1], rotMtx[2][1], rotMtx[3][1],
		rotMtx[0][2], rotMtx[1][2], rotMtx[2][2], rotMtx[3][2],
		rotMtx[0][3], rotMtx[1][3], rotMtx[2][3], rotMtx[3][3]);

	invT = invT * invRotMtx;



}

void OrientablePrimitive::translate(float dx, float dy, float dz){

	//T^-1 = T^-1 * Translate^-1 
	invT[0][3] = invT[0][3] - (dx*invT[0][0] + dy*invT[0][1] + dz*invT[0][2]);
	invT[1][3] = invT[1][3] - (dx*invT[1][0] + dy*invT[1][1] + dz*invT[1][2]);
	invT[2][3] = invT[2][3] - (dx*invT[2][0] + dy*invT[2][1] + dz*invT[2][2]);
	invT[3][3] = invT[3][3] - (dx*invT[3][0] + dy*invT[3][1] + dz*invT[3][2]);



}

void OrientablePrimitive::scale(float a, float b, float c){

	if (a == 0) a = 1.0;
	if (b == 0) b = 1.0;
	if (c == 0) c = 1.0;

	invT[0][0] = invT[0][0] * (1.0 / a); invT[0][1] = invT[0][1] * (1.0 / b); invT[0][2] = invT[0][2] * (1.0 / c);
	invT[1][0] = invT[1][0] * (1.0 / a); invT[1][1] = invT[1][1] * (1.0 / b); invT[1][2] = invT[1][2] * (1.0 / c);
	invT[2][0] = invT[2][0] * (1.0 / a); invT[2][1] = invT[2][1] * (1.0 / b); invT[2][2] = invT[2][2] * (1.0 / c);
	invT[3][0] = invT[3][0] * (1.0 / a); invT[3][1] = invT[3][1] * (1.0 / b); invT[3][2] = invT[3][2] * (1.0 / c);

}
//////////////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere() :Primitive(){

	Sphere::position = Vector3f(0.0, 0.0, 0.0);
	Sphere::radius = 1;

}

Sphere::Sphere(Vector3f position, float radius, Color color) :Primitive(color){
	Sphere::position = position;
	Sphere::radius = radius;

}

Sphere::~Sphere(){}



bool Sphere::hit(const Ray &ray, Hit &hit) {

	//Use position - origin to get a negative b
	Vector3f L = position - ray.origin;


	float b = Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - radius*radius;

	float d = b*b - c;
	if (d < 0.00001) return false;

	//----------------------------------
	float result = -1.0;

	result = b - sqrt(d);
	if (result < 0.0){

		result = b + sqrt(d);

	}


	if (result > 0.0){

		hit.t = result;
		return true;

	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle() :Primitive(){

	Triangle::normal = Vector3f(0.0, 0.0, 1.0);
	Triangle::a = Vector3f(-1.0, 0.0, 0.0);
	Triangle::b = Vector3f(0.0, 3.0, 0.0);
	Triangle::c = Vector3f(1.0, 0.0, 0.0);
}

Triangle::Triangle(const Vector3f &normal, const Vector3f &a, const Vector3f &b, const Vector3f &c, Color color) :Primitive(color){
	Triangle::normal = normal;
	Triangle::a = a;
	Triangle::b = b;
	Triangle::c = c;

}

Triangle::~Triangle(){}

// Möller Trumbore algorithm
bool Triangle::hit(const Ray &ray, Hit &hit){


	//determinat of the triangle

	Vector3f v0v1 = Triangle::b - Triangle::a;
	Vector3f v0v2 = Triangle::c - Triangle::a;

	Vector3f P = Vector3f::cross(ray.direction, v0v2);
	float det = Vector3f::dot(P, v0v1);

	if (fabs(det) < 0.000001) return false;

	float inv_det = 1.0 / det;

	// Barycentric Coefficients 

	Vector3f T = ray.origin - Triangle::a;
	// the intersection lies outside of the triangle
	float u = Vector3f::dot(T, P) * inv_det;
	if (u < 0.0 || u > 1.0) return false;

	// prepare to test v parameter
	Vector3f Q = Vector3f::cross(T, v0v1);

	// the intersection is outside the triangle
	float v = Vector3f::dot(ray.direction, Q) * inv_det;
	if (v < 0 || u + v > 1) return false;

	float result = -1.0;
	result = Vector3f::dot(v0v2, Q) * inv_det;

	if (result > 0.0){
		hit.t = result;
		return true;
	}
	return false;

}


//////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::normal = Vector3f(0.0, 1.0, 0.0);
	Plane::distance = -1.0;

}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitive(color){
	Plane::normal = normal;
	Plane::distance = distance;

}

Plane::~Plane(){}

bool Plane::hit(const Ray &ray, Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(normal, ray.origin)) / Vector3f::dot(normal, ray.direction);

	if (result > 0.0){
		hit.t = result;
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Torus::Torus() :OrientablePrimitive(){

	Torus::a = 1.0;
	Torus::b = 0.5;
}

Torus::Torus(float a, float b, Color color) :OrientablePrimitive(color){

	Torus::a = a;
	Torus::b = b;
}

Torus::~Torus(){

}

bool Torus::hit(const Ray &ray, Hit &hit){

	float Ra2 = Torus::a*Torus::a;
	float ra2 = Torus::b*Torus::b;


	float m = Vector3f::dot(ray.origin, ray.origin);
	float n = Vector3f::dot(ray.origin, ray.direction);

	float k = (m - ra2 - Ra2) / 2.0;
	float a = n;
	float b = n*n + Ra2*ray.direction[0] * ray.direction[0] + k;
	float c = k*n + Ra2*ray.origin[0] * ray.direction[0];
	float d = k*k + Ra2*ray.origin[0] * ray.origin[0] - Ra2*ra2;

	//----------------------------------

	float p = -3.0*a*a + 2.0*b;
	float q = 2.0*a*a*a - 2.0*a*b + 2.0*c;
	float r = -3.0*a*a*a*a + 4.0*a*a*b - 8.0*a*c + 4.0*d;
	p /= 3.0;
	r /= 3.0;
	float Q = p*p + r;
	float R = 3.0*r*p - p*p*p - q*q;

	float h = R*R - Q*Q*Q;
	float z = 0.0;

	// discriminant > 0
	if (h < 0.0)
	{
		float sQ = sqrt(Q);
		z = 2.0*sQ*cos(acos(R / (sQ*Q)) / 3.0);
	}
	// discriminant < 0
	else
	{
		float sQ = pow(sqrt(h) + abs(R), 1.0 / 3.0);
		if (R < 0.0){
			z = -(sQ + Q / sQ);
		}
		else{
			z = (sQ + Q / sQ);
		}
	}

	z = p - z;

	//----------------------------------

	float d1 = z - 3.0*p;
	float d2 = z*z - 3.0*r;
	float d1o2 = d1 / 2.0;

	if (abs(d1)< 0.0001)
	{
		if (d2<0.0) return false;
		d2 = sqrt(d2);
	}
	else
	{
		if (d1<0.0) return false;
		d1 = sqrt(d1 / 2.0);
		d2 = q / d1;
	}

	//----------------------------------

	float result = -1.0;
	float result2 = -1.0;

	float t1;
	float t2;

	h = d1o2 - z + d2;
	if (h>0.0)
	{
		h = sqrt(h);

		t1 = -d1 - h - a;
		t2 = -d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result = std::min(t1, t2);
		}
		else if (t1 > 0) {
			result = t1;
		}
		else if (t2 > 0){
			result = t2;
		}

	}


	h = d1o2 - z - d2;
	if (h>0.0)
	{
		h = sqrt(h);
		t1 = d1 - h - a;
		t2 = d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result2 = std::min(t1, t2);
		}
		else if (t1 > 0) {
			result2 = t1;
		}
		else if (t2 > 0){
			result2 = t2;
		}
	}

	if (result2 > 0.0 && result > 0.0){
		result = std::min(result, result2);

	}
	else if (result2 > 0.0) result = result2;



	if (result > 0.0){
		hit.t = result;
		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////
Mesh::Mesh(){}
Mesh::~Mesh(){

	for (int i = 0; i < triangles.size(); i++){
		delete triangles[i];

	}

}

struct face{
	int facenum;
	bool four;
	int faces[4];
	face(int facen, int f1, int f2, int f3) :facenum(facen){
		faces[0] = f1;
		faces[1] = f2;
		faces[2] = f3;
		four = false;
	}

	face(int facen, int f1, int f2, int f3, int f4) :facenum(facen){
		faces[0] = f1;
		faces[1] = f2;
		faces[2] = f3;
		faces[3] = f4;
		four = true;
	}
};

bool Mesh::loadObject(const char* filename){


	std::vector<std::string*>coord;
	std::vector<Vector3f*> vertex;
	std::vector<face*> faces;
	std::vector<Vector3f*> normals;
	std::ifstream in(filename);

	if (!in.is_open()){

		std::cout << "Not opend" << std::endl;
		return false;
	}


	std::string line;
	while (getline(in, line)){
		coord.push_back(new std::string(line));

	}
	in.close();

	for (int i = 0; i < coord.size(); i++){
		if ((*coord[i])[0] == '#')
			continue;
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == ' '){


			float tmpx, tmpy, tmpz;
			sscanf(coord[i]->c_str(), "v %f %f %f", &tmpx, &tmpy, &tmpz);
			vertex.push_back(new Vector3f(tmpx, tmpy, tmpz));
		}
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == 'n'){
			float tmpx, tmpy, tmpz;
			sscanf(coord[i]->c_str(), "vn %f %f %f", &tmpx, &tmpy, &tmpz);
			normals.push_back(new Vector3f(tmpx, tmpy, tmpz));
		}
		else if ((*coord[i])[0] == 'f'){

			int a, b, c, d, e;

			if (std::count(coord[i]->begin(), coord[i]->end(), ' ') == 3){

				sscanf(coord[i]->c_str(), "f %d/%d %d/%d %d/%d", &a, &b, &c, &b, &d, &b);
				faces.push_back(new face(b, a, c, d));
			}
			else{
				sscanf(coord[i]->c_str(), "f %d/%d %d/%d %d/%d %d/%d", &a, &b, &c, &b, &d, &b, &e, &b);
				faces.push_back(new face(b, a, c, d, e));
			}

		}
	}


	Triangle *triangle;

	for (int i = 0; i < faces.size(); i++){
		triangle = new Triangle(*normals[faces[i]->facenum - 1], *vertex[faces[i]->faces[0] - 1], *vertex[faces[i]->faces[1] - 1], *vertex[faces[i]->faces[2] - 1], Color(0.3, 0.3, 0.3));
		triangles.push_back(triangle);

	}

	for (int i = 0; i < coord.size(); i++){
		delete coord[i];

	}
	for (int i = 0; i < faces.size(); i++){
		delete faces[i];
	}
	for (int i = 0; i < normals.size(); i++){
		delete normals[i];
	}
	for (int i = 0; i < vertex.size(); i++){
		delete vertex[i];
	}


	return true;
}