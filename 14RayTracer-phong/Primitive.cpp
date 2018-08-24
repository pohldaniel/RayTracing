#include <array>
#include "Model.h"


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

	Primitive::m_color = Color(0.0, 1.0, 0.0);

	Primitive::orientable = false;
	Primitive::bounds = false;
	Primitive::T.identity();
	Primitive::invT.identity();
	Primitive::box = BBox(Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	Primitive::m_texture = NULL;
	Primitive::m_material = NULL;

}

Primitive::Primitive(const Color &color){

	Primitive::m_color = color;
	Primitive::orientable = false;
	Primitive::bounds = false;
	Primitive::T.identity();
	Primitive::invT.identity();
	Primitive::box = BBox(Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	Primitive::m_texture = NULL;
	Primitive::m_material = NULL;
	
}

Primitive::~Primitive(){
	if (m_texture){

		delete m_texture;
		m_texture = NULL;
	}

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

void Primitive::setTexture(Texture* texture){

	Primitive::m_texture = texture;
}

Texture* Primitive::getTexture(){
	return m_texture;
}

void Primitive::setMaterial(Material* material){

	Primitive::m_material = material;
}

Material* Primitive::getMaterial(){
	return m_material;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
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
	rotMtx.invRotate(axis, degrees);


	Matrix4f invRotMtx = Matrix4f(rotMtx[0][0], rotMtx[1][0], rotMtx[2][0], rotMtx[3][0],
								  rotMtx[0][1], rotMtx[1][1], rotMtx[2][1], rotMtx[3][1],
							      rotMtx[0][2], rotMtx[1][2], rotMtx[2][2], rotMtx[3][2],
								  rotMtx[0][3], rotMtx[1][3], rotMtx[2][3], rotMtx[3][3]);

	T = rotMtx * T;
	invT = invT * invRotMtx;
	
}

void OrientablePrimitive::translate(float dx, float dy, float dz){

	//T = Translate * T 
	T[0][0] = T[0][0] - T[3][0] * dx; T[1][0] = T[1][0] - T[3][0] * dy; T[2][0] = T[2][0] - T[3][0] * dz;
	T[0][1] = T[0][1] - T[3][1] * dx; T[1][1] = T[1][1] - T[3][1] * dy; T[2][1] = T[2][1] - T[3][1] * dz;
	T[0][2] = T[0][2] - T[3][2] * dx; T[1][2] = T[1][2] - T[3][2] * dy; T[2][2] = T[2][2] - T[3][2] * dz;
	T[0][3] = T[0][3] + T[3][3] * dx; T[1][3] = T[1][3] + T[3][3] * dy; T[2][3] = T[2][3] + T[3][3] * dz;


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

	T[0][0] = T[0][0] * a;  T[1][0] = T[1][0] * b; T[2][0] = T[2][0] * c;
	T[0][1] = T[0][1] * a;  T[1][1] = T[1][1] * b; T[2][1] = T[2][1] * c;
	T[0][2] = T[0][2] * a;  T[1][2] = T[1][2] * b; T[2][2] = T[2][2] * c;
	T[0][3] = T[0][3] * a;  T[1][3] = T[1][3] * b; T[2][3] = T[2][3] * c;

	invT[0][0] = invT[0][0] * (1.0 / a); invT[0][1] = invT[0][1] * (1.0 / b); invT[0][2] = invT[0][2] * (1.0 / c);
	invT[1][0] = invT[1][0] * (1.0 / a); invT[1][1] = invT[1][1] * (1.0 / b); invT[1][2] = invT[1][2] * (1.0 / c);
	invT[2][0] = invT[2][0] * (1.0 / a); invT[2][1] = invT[2][1] * (1.0 / b); invT[2][2] = invT[2][2] * (1.0 / c);
	invT[3][0] = invT[3][0] * (1.0 / a); invT[3][1] = invT[3][1] * (1.0 / b); invT[3][2] = invT[3][2] * (1.0 / c);


}
//////////////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle(Vector3f &a_V1,
	Vector3f &a_V2,
	Vector3f &a_V3,
	Color	&color) : OrientablePrimitive(color){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	abc = Vector3f::cross(m_a - m_b, m_a - m_c).magnitude();
	calcBounds();
	m_hasnormal = false;
	
}




Triangle::~Triangle(){


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

	
	if (fabs(det) <0.000001 ) return;
	


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
	if (v < 0 || u + v > 1) return;

	float result = -1.0;
	result = Vector3f::dot(v0v2, Q) * inv_det;

	if (result > 0.0){

		hit.t = result;
		hit.hitObject = true;
	}
	return;

}

Color Triangle::getColor(Vector3f& pos){
	
	if (m_texture){



		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;

		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;

		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;
		

		double u = u1*d1 + u2*d2 + u3*d3;
		double v = v1*d1 + v2*d2 + v3*d3;


		Color color = m_texture->getTexel(u, v) * Color(0.2, 0.2, 0.2);

		return  color;

	}
	else{

		return m_color;

	}
}

Vector3f Triangle::getNormal(Vector3f& pos){
	
	
	if (m_hasnormal){

		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;

		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;

		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

		Vector3f normal =  (m_n1 * d1 + m_n2 * d2 + m_n3 * d3).normalize();


		return  normal;

	}else{


		return Vector3f(1.0, 0.0, 0.0);
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere(Vector3f& a_Centre, double a_Radius, Color color) :Primitive(color){

	m_Centre = a_Centre;
	m_SqRadius = a_Radius * a_Radius;
	m_Radius = a_Radius;
	m_RRadius = 1.0f / a_Radius;
	calcBounds();
}

Sphere::~Sphere(){

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
	float delta = 0.00001;

	box.extend(m_Centre - Vector3f(m_Radius + delta, m_Radius + delta, m_Radius + delta));
	box.extend(m_Centre + Vector3f(m_Radius * 2 + delta, m_Radius * 2 + delta, m_Radius * 2 + delta));
	Sphere::bounds = true;
}

Color Sphere::getColor(Vector3f& pos){

	if (m_texture){

		Vector3f vp = (pos - m_Centre) * m_RRadius;

		float theta = acos(vp[1]);

		float phi = atan2(vp[0], vp[2]);
		if (phi < 0.0)
			phi += TWO_PI;

		// next, map theta and phi to (u, v) in [0, 1] X [0, 1]

		float u = (phi * invTWO_PI);
		float v = (1.0 - theta * invPI);


		Color color = m_texture->getTexel(u, v) * Color(0.2f, 0.2f, 0.2f);

		return color;

	}
	else {

		return m_color;
	}
}

Vector3f Sphere::getNormal(Vector3f& a_Pos){

	return ((a_Pos - m_Centre) * m_RRadius).normalize();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::distance = -1.0;
	calcBounds();
}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitive(color){

	Plane::distance = distance;
	Plane::m_normal = normal;

	//rotate normal 90 degree arround z-axis
	m_u = Vector3f(normal[1], -normal[0], -normal[2]);
	m_v = Vector3f::cross(normal, m_u);
	calcBounds();
}

void Plane::hit(const Ray &ray, Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(m_normal, ray.origin)) / Vector3f::dot(m_normal, ray.direction);

	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

void Plane::calcBounds(){

	Plane::bounds = false;

}

Color Plane::getColor(Vector3f& a_Pos){

	if (m_texture){

		float u = abs(Vector3f::dot(a_Pos, m_u));
		float v = abs(Vector3f::dot(a_Pos, m_v));

		Color color = m_texture->getTexel(u, v) * Color(0.2f, 0.2f, 0.2f);

		return color;

	}
	else{

		return m_color;
	}
}

Vector3f Plane::getNormal(Vector3f& a_Pos){

	return  m_normal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

Torus::Torus() :OrientablePrimitive(){

	Torus::a = 1.0;
	Torus::b = 0.5;
	calcBounds();
}

Torus::Torus(float a, float b, Color color) :OrientablePrimitive(color){

	Torus::a = a;
	Torus::b = b;
	calcBounds();

}

Torus::~Torus(){

}

void Torus::hit(const Ray &_ray, Hit &hit){

	/*if (!box.intersect(_ray)){
		hit.hitObject = false;
		return;
	}*/

	Vector3f ro = _ray.origin;
	Vector3f rd = _ray.direction;

	double Ra2 = Torus::a*Torus::a;
	double ra2 = Torus::b*Torus::b;


	double m = Vector3f::dot(ro, ro);
	double n = Vector3f::dot(ro, rd);

	double k = (m - ra2 - Ra2) / 2.0;
	double a = n;
	double b = n*n + Ra2*rd[0] * rd[0] + k;
	double c = k*n + Ra2*ro[0] * rd[0];
	double d = k*k + Ra2*ro[0] * ro[0] - Ra2*ra2;

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

	float delta = 0.00001;

	box.m_Pos[0] = -(b + delta);
	box.m_Pos[1] = -(a + b + delta);
	box.m_Pos[2] = -(a + b + delta);

	box.m_Size[0] = 2 * b + delta;
	box.m_Size[1] = 2 * (a + b) + delta;
	box.m_Size[2] = 2 * (a + b) + delta;

	Torus::bounds = true;
	
}

Color Torus::getColor(Vector3f& a_pos){

	if (m_texture){

		// Determine its angle from the x-axis.
		float u = (1.0 - (atan2(a_pos[2], a_pos[1]) + PI) / TWO_PI);

		float len = sqrt(a_pos[1] * a_pos[1] + a_pos[2] * a_pos[2]);


		// Now rotate about the x-axis to get the point P into the x-z plane.
		float x = len - a;
		float v = ((atan2(a_pos[0], x) + PI) / TWO_PI);

		Color color = m_texture->getTexel(u, v) * Color(0.2f, 0.2f, 0.2f);

		return color;

	}
	else{

		return m_color;
	}

}

Vector3f Torus::getNormal(Vector3f& a_pos){

	// calculate the normal like http://cosinekitty.com/raytrace/chapter13_torus.html

	Vector3f normal;
	Vector3f tmp;

	float dist = sqrtf(a_pos[1] * a_pos[1] + a_pos[2] * a_pos[2]);

	if (dist > 0.0001){
		
		tmp[0] = 0.0;
		tmp[1] = a *a_pos[1] / dist;
		tmp[2] = a *a_pos[2] / dist;

	}
	else{

		tmp[0] = 0.0;
		tmp[1] = 0.0;
		tmp[2] = 0.0;
	}

	normal = a_pos - tmp;
	
	//calculate the transpose of invT with the normal
	return (invT * normal).normalize();

	//return (invT.transpose() * Vector4f(normal, 0.0)).normalize();
	//return (T * Vector4f(normal, 0.0)).normalize();

	// calculate the normal with the gradient df(x)/dx

	/*Vector3f normal;

	float param_squared = a * a + b * b;
	float sum_squared = a_pos[0] * a_pos[0] + a_pos[1] * a_pos[1] + a_pos[2] * a_pos[2];

	normal[0] =  a_pos[0] * (sum_squared - param_squared + a * a);
	normal[1] =  a_pos[1] * (sum_squared - param_squared );
	normal[2] =  a_pos[2] * (sum_squared - param_squared );

	//calculate the transpose of invT with the normal
	return (invT * normal).normalize();

	//return (invT.transpose() * Vector4f(normal, 0.0)).normalize();
	//return (T * Vector4f(normal, 0.0)).normalize();*/

}
//////////////////////////////////////////////////////////////////////////////////////////////////////







