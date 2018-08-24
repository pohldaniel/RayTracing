#include <array>
#include "KDTree.h"


bool BBox::intersect(const Ray& ray) {
	double ox = ray.origin[0]; double oy = ray.origin[1]; double oz = ray.origin[2];
	double dx = ray.direction[0]; double dy = ray.direction[1]; double dz = ray.direction[2];

	double tx_min, ty_min, tz_min;
	double tx_max, ty_max, tz_max;

	double a = 1.0 / dx;
	if (a >= 0) {
		tx_min = (m_Pos[0] - ox) * a;
		tx_max = (m_Size[0] + m_Pos[0] - ox) * a;
	}
	else {
		tx_min = (m_Size[0] + m_Pos[0] - ox) * a;
		tx_max = (m_Pos[0] - ox) * a;
	}

	double b = 1.0 / dy;
	if (b >= 0) {
		ty_min = (m_Pos[1] - oy) * b;
		ty_max = (m_Size[1] + m_Pos[1] - oy) * b;
	}
	else {
		ty_min = (m_Size[1] + m_Pos[1] - oy) * b;
		ty_max = (m_Pos[1] - oy) * b;
	}

	double c = 1.0 / dz;
	if (c >= 0) {
		tz_min = (m_Pos[2] - oz) * c;
		tz_max = (m_Size[2] + m_Pos[2] - oz) * c;
	}
	else {
		tz_min = (m_Size[2] + m_Pos[2] - oz) * c;
		tz_max = (m_Pos[2] - oz) * c;
	}

	double t0, t1;

	// find largest entering t value

	if (tx_min > ty_min)
		t0 = tx_min;
	else
		t0 = ty_min;

	if (tz_min > t0)
		t0 = tz_min;

	// find smallest exiting t value

	if (tx_max < ty_max)
		t1 = tx_max;
	else
		t1 = ty_max;

	if (tz_max < t1)
		t1 = tz_max;

	if (t0 < t1 && t1 > 0.0001){
		m_tmin = t0;
		m_tmax = t1;
		return true;
	}
	return false;

}
//////////////////////////////////////////////////////////////////////////////////////////////////

Primitive::Primitive() {

	Primitive::color = Color(0.0, 1.0, 0.0);
	Primitive::normal = Vector3f(0.0, 1.0, 0.0);
	Primitive::orientable = false;
	Primitive::bounds = false;
	Primitive::invT.identity();
	Primitive::box = BBox(Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}

Primitive::Primitive(const Color &color, const Vector3f &normal){

	Primitive::color = color;
	Primitive::normal = normal;
	Primitive::orientable = false;
	Primitive::bounds = false;
	Primitive::invT.identity();
	Primitive::box = BBox(Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}

Primitive::~Primitive(){

}

BBox &Primitive::getBounds(){

	if (!bounds){

		calcBounds();

	}
	return box;
}

void Primitive::clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox)
{
	//clearing the boxes
	leftBoundingBox = getBounds();
	rightBoundingBox = getBounds();
	leftBoundingBox.getSize()[axis] = position;
	rightBoundingBox.getPos()[axis] = position;

}


//////////////////////////////////////////////////////////////////////////////////////////////////
OrientablePrimitive::OrientablePrimitive() :Primitive(){
	orientable = true;

}


OrientablePrimitive::OrientablePrimitive(const Color& color, const Vector3f &normal) : Primitive(color, normal){
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
//////////////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle() {

}

Triangle::Triangle(Vector3f &a_V1, 
				   Vector3f &a_V2, 
				   Vector3f &a_V3, 
				   Vector3f &normal) : OrientablePrimitive(Color(1.0, 0.0, 0.0), normal)
{
	
	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	Triangle::normal.normalize();

}




Triangle::~Triangle()
{

}

void Triangle::calcBounds(){
	
	float delta = 0.000001;
	box.m_Pos[0] = min(min(m_a[0], m_b[0]), m_c[0]) - delta;
	box.m_Pos[1] = min(min(m_a[1], m_b[1]), m_c[1]) - delta;
	box.m_Pos[2] = min(min(m_a[2], m_b[2]), m_c[2]) - delta;

	box.m_Size[0] = max(max(m_a[0], m_b[0]), m_c[0]) + delta;
	box.m_Size[1] = max(max(m_a[1], m_b[1]), m_c[1]) + delta;
	box.m_Size[2] = max(max(m_a[2], m_b[2]), m_c[2]) + delta;

	Triangle::bounds = true;
}



// Möller Trumbore algorithm
void Triangle::hit(const Ray &ray, Hit &hit){
	
	
	//determinat of the triangle
	Vector3f v0v1 = Triangle::m_b - Triangle::m_a;
	Vector3f v0v2 = Triangle::m_c - Triangle::m_a;


	Vector3f P = Vector3f::cross(ray.direction, v0v2);
	float det = Vector3f::dot(P, v0v1);

	if (fabs(det) < 0.000001) return;
	
	float inv_det = 1.0 / det;

	// Barycentric Coefficients 
	Vector3f T = ray.origin - Triangle::m_a;

	// the intersection lies outside of the triangle
	float u = Vector3f::dot(T, P) * inv_det;

	//std::cout << u << std::endl;
	if (u < 0.0 || u > 1.0) return;
	
	// prepare to test v parameter
	Vector3f Q = Vector3f::cross(T, v0v1);

	// the intersection is outside the triangle
	float v = Vector3f::dot(ray.direction, Q) * inv_det;
	if (v < 0 || u + v > 1) return ;

	float result = -1.0;
	result = Vector3f::dot(v0v2, Q) * inv_det;
	
	if (result > 0.0){
		
		hit.t = result;
		hit.hitObject = true;
	}
	return ;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
Sphere::Sphere(){

}

Sphere::Sphere(Vector3f& a_Centre, double a_Radius, Color color) :Primitive(color , Vector3f(0.0, 1.0, 0.0) )
{
	m_Centre = a_Centre;
	m_SqRadius = a_Radius * a_Radius;
	m_Radius = a_Radius;
	m_RRadius = 1.0f / a_Radius;
}

Sphere::~Sphere(){

}

Vector3f Sphere::getNormal(Vector3f& a_Pos)
{
		return (a_Pos - m_Centre) * m_RRadius;
}

void Sphere::hit(const Ray &ray, Hit &hit) {

	//Use position - origin to get a negative b
	Vector3f L = m_Centre - ray.origin;


	float b = Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - m_SqRadius;

	float d = b*b - c;
	if (d < 0.00001) {
		hit.hitObject = false;
		return;
	}
	//----------------------------------
	float result = -1.0;

	result = b - sqrt(d);
	if (result < 0.0){

		result = b + sqrt(d);

	}


	if (result > 0.0){

		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

void Sphere::calcBounds(){

	box.extend(m_Centre - Vector3f(m_Radius, m_Radius, m_Radius));
	box.extend(m_Centre + Vector3f(m_Radius, m_Radius, m_Radius));
	Sphere::bounds = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::normal = Vector3f(0.0, 1.0, 0.0);
	Plane::distance = -1.0;

}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitive(color, normal){

	Plane::distance = distance;

}

void Plane::hit(const Ray &ray, Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(normal, ray.origin)) / Vector3f::dot(normal, ray.direction);

	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

void Plane::calcBounds(){

	return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

Torus::Torus() :OrientablePrimitive(){

	Torus::a = 1.0;
	Torus::b = 0.5;
}

Torus::Torus(float a, float b, Color color) :OrientablePrimitive(color, Vector3f(0.0 , 1.0 ,0.0)){

	Torus::a = a;
	Torus::b = b;
}

Torus::~Torus(){

}

void Torus::hit(const Ray &ray, Hit &hit){

	double Ra2 = Torus::a*Torus::a;
	double ra2 = Torus::b*Torus::b;


	double m = Vector3f::dot(ray.origin, ray.origin);
	double n = Vector3f::dot(ray.origin, ray.direction);

	double k = (m - ra2 - Ra2) / 2.0;
	double a = n;
	double b = n*n + Ra2*ray.direction[0] * ray.direction[0] + k;
	double c = k*n + Ra2*ray.origin[0] * ray.direction[0];
	double d = k*k + Ra2*ray.origin[0] * ray.origin[0] - Ra2*ra2;

	//----------------------------------

	double p = -3.0*a*a + 2.0*b;
	double q = 2.0*a*a*a - 2.0*a*b + 2.0*c;
	double r = -3.0*a*a*a*a + 4.0*a*a*b - 8.0*a*c + 4.0*d;
	p /= 3.0;
	r /= 3.0;
	double Q = p*p + r;
	double R = 3.0*r*p - p*p*p - q*q;

	double h = R*R - Q*Q*Q;
	double z = 0.0;

	// discriminant > 0
	if (h < 0.0)
	{
		double sQ = sqrt(Q);
		z = 2.0*sQ*cos(acos(R / (sQ*Q)) / 3.0);
	}
	// discriminant < 0
	else
	{
		double sQ = pow(sqrt(h) + abs(R), 1.0 / 3.0);
		if (R < 0.0){
			z = -(sQ + Q / sQ);
		}
		else{
			z = (sQ + Q / sQ);
		}
	}

	z = p - z;

	//----------------------------------

	double d1 = z - 3.0*p;
	double d2 = z*z - 3.0*r;
	double d1o2 = d1 / 2.0;

	if (abs(d1)< 0.0001)
	{
		if (d2 < 0.0)  {
			hit.hitObject = false;
			return;
		}
		d2 = sqrt(d2);
	}
	else
	{
		if (d1 < 0.0) {
			hit.hitObject = false;
			return;
		}
		d1 = sqrt(d1 / 2.0);
		d2 = q / d1;
	}

	//----------------------------------

	double result = -1.0;
	double result2 = -1.0;

	double t1;
	double t2;

	h = d1o2 - z + d2;
	if (h>0.0)
	{
		h = sqrt(h);

		t1 = -d1 - h - a;
		t2 = -d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result = min(t1, t2);
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
			result2 = min(t1, t2);
		}
		else if (t1 > 0) {
			result2 = t1;
		}
		else if (t2 > 0){
			result2 = t2;
		}
	}

	if (result2 > 0.0 && result > 0.0){
		result = min(result, result2);

	}
	else if (result2 > 0.0) result = result2;



	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

void Torus::calcBounds(){

	return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh() {

	Mesh::color = Color(1.0, 1.0, 0.0);
	Mesh::xmin = FLT_MAX;
	Mesh::ymin = FLT_MAX;
	Mesh::zmin = FLT_MAX;
	Mesh::xmax = -FLT_MAX;
	Mesh::ymax = -FLT_MAX;
	Mesh::zmax = -FLT_MAX;
}




Mesh::~Mesh(){

}


void Mesh::hit(const Ray& a_Ray, Hit &hit){
	
	// find the nearest intersection
	m_KDTree->intersectRec(a_Ray, hit);
	
}

bool Mesh::loadObject(const char* filename){
	
	return loadObject(filename, 1.0);
}

void Mesh::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin); 
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	Mesh::bounds = true;
}


bool Mesh::loadObject(const char* filename, float scale){

	std::vector<std::string*>coord;
	std::vector<Vector3f*> vertex;
	std::vector<std::array<int, 5>> vec;
	std::vector<Vector3f*> normals;
	std::ifstream in(filename);

	if (!in.is_open()){

		std::cout << "File not found" << std::endl;
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
			vertex.push_back(new Vector3f(tmpx*scale, tmpy * scale, tmpz * scale));
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

				vec.push_back({ { b, a, c, d, 0 } });
			}
			else{

				sscanf(coord[i]->c_str(), "f %d/%d %d/%d %d/%d ", &a, &d, &b, &d, &c, &d);
			
				vec.push_back({ { a, b, c, d, 0 } });

			}

		}
	}

	Vector3f *normal;
	Vector3f *a;
	Vector3f *b;
	Vector3f *c;
	Triangle *triangle;

	for (int i = 0; i < vec.size(); i++){	

			a = vertex[(vec[i])[0] - 1];
			b = vertex[(vec[i])[1] - 1];
			c = vertex[(vec[i])[2] - 1];

			normal = normals[(vec[i])[3] - 1];

			xmin = min(a->getVec()[0], min(b->getVec()[0], min(c->getVec()[0], xmin)));
			ymin = min(a->getVec()[1], min(b->getVec()[1], min(c->getVec()[1], ymin)));
			zmin = min(a->getVec()[2], min(b->getVec()[2], min(c->getVec()[2], zmin)));

			xmax = max(a->getVec()[0], max(b->getVec()[0], max(c->getVec()[0], xmax)));
			ymax = max(a->getVec()[1], max(b->getVec()[1], max(c->getVec()[1], ymax)));
			zmax = max(a->getVec()[2], max(b->getVec()[2], max(c->getVec()[2], zmax)));

			triangle = new Triangle(*a, *b, *c, Vector3f(0.0, 0.0, 1.0));

			triangles.push_back(triangle);
	}
	std::cout <<"Number of faces: " << vec.size() << std::endl;
	calcBounds();
	std::cout << "Build KDTree!" << std::endl;

	m_KDTree = new KDTree();
	m_KDTree->buildTree(triangles, box);

	std::cout << "Finished KDTree!" << std::endl;

	for (int i = 0; i < coord.size(); i++){
		delete coord[i];

	}
	
	for (int i = 0; i < normals.size(); i++){
		delete normals[i];
	}
	for (int i = 0; i < vertex.size(); i++){
		delete vertex[i];
	}

	return true;
}








