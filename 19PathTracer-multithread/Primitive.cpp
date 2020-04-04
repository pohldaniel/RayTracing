#include <array>
#include <random>
#include "Model.h"

bool BBox::intersect(const Ray& a_ray) {
	double ox = a_ray.origin[0]; double oy = a_ray.origin[1]; double oz = a_ray.origin[2];
	double dx = a_ray.direction[0]; double dy = a_ray.direction[1]; double dz = a_ray.direction[2];

	double tx_min, ty_min, tz_min;
	double tx_max, ty_max, tz_max;

	double a = 1.0 / dx;
	if (a >= 0) {
		tx_min = (m_pos[0] - ox) * a;
		tx_max = (m_size[0] + m_pos[0] - ox) * a;
	}else {
		tx_min = (m_size[0] + m_pos[0] - ox) * a;
		tx_max = (m_pos[0] - ox) * a;
	}

	double b = 1.0 / dy;
	if (b >= 0) {
		ty_min = (m_pos[1] - oy) * b;
		ty_max = (m_size[1] + m_pos[1] - oy) * b;
	}else {
		ty_min = (m_size[1] + m_pos[1] - oy) * b;
		ty_max = (m_pos[1] - oy) * b;
	}

	double c = 1.0 / dz;
	if (c >= 0) {
		tz_min = (m_pos[2] - oz) * c;
		tz_max = (m_size[2] + m_pos[2] - oz) * c;
	}else {
		tz_min = (m_size[2] + m_pos[2] - oz) * c;
		tz_max = (m_pos[2] - oz) * c;
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

	m_color = Color(0.0, 0.0, 1.0);

	bounds = false;
	
	box = BBox(Vector3f(FLT_MAX, FLT_MAX, FLT_MAX), Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	m_texture = NULL;
	m_material = NULL;
	m_useTexture = true;
	
}

Primitive::~Primitive(){}

BBox &Primitive::getBounds(){

	if (!bounds){

		calcBounds();
	}
	return box;
}

void Primitive::clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox){
	//clearing the boxes
	leftBoundingBox = getBounds();
	rightBoundingBox = getBounds();
	leftBoundingBox.getSize()[axis] = position;
	rightBoundingBox.getPos()[axis] = position;
}


void Primitive::setTexture(Texture* texture){
	
	if (texture == NULL){
		m_useTexture = false;
		m_texture = NULL;
		
	}else{
		
		m_texture = std::shared_ptr<Texture>(texture);
		
	}
}

std::shared_ptr<Texture> Primitive::getTexture(){
	
	return m_texture;
}

void Primitive::setMaterial(Material* material){

	m_material =  std::shared_ptr<Material>(material);
}

std::shared_ptr<Material> Primitive::getMaterial(){

	return m_material;
}

void Primitive::setColor(Color color){

	m_color = color;
}

Color Primitive::getColor(const Vector3f& pos){
	
	if (m_texture){
		
		if (m_texture->getProcedural()){

			return static_cast<ProceduralTexture*>(m_texture.get())->getColor(pos);

		}else{

			std::pair <float, float> uv = getUV(pos);
			return static_cast<ImageTexture*>(m_texture.get())->getTexel(uv.first, uv.second, pos);
		}

	}else {
		
		return m_color;
	}
}

Vector3f Primitive::getNormalDu(const Vector3f& pos){
	
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Primitive::getNormalDv(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Primitive::sample(void){

	return Vector3f(0.0, 0.0, 0.0);
}

float Primitive::pdf(Hit &hit){
	return 0.0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Instance::Instance(Primitive *primitive){

	T.identity();
	invT.identity();

	
	m_texture = NULL;
	m_material = NULL;

	m_color = primitive->m_color;
	m_defaultColor = true;
	m_useTexture = primitive->m_useTexture;
	m_primitive = std::shared_ptr<Primitive>(primitive);
	
}

Instance::~Instance(){

}

void Instance::rotate(const Vector3f &axis, float degrees){

	Matrix4f rotMtx;
	rotMtx.invRotate(axis, degrees);


	Matrix4f invRotMtx = Matrix4f(rotMtx[0][0], rotMtx[1][0], rotMtx[2][0], rotMtx[3][0],
		rotMtx[0][1], rotMtx[1][1], rotMtx[2][1], rotMtx[3][1],
		rotMtx[0][2], rotMtx[1][2], rotMtx[2][2], rotMtx[3][2],
		rotMtx[0][3], rotMtx[1][3], rotMtx[2][3], rotMtx[3][3]);

	T = rotMtx * T;
	invT = invT * invRotMtx;

}

void Instance::translate(float dx, float dy, float dz){

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

void Instance::scale(float a, float b, float c){

	if (a == 0) a = 1.0f;
	if (b == 0) b = 1.0f;
	if (c == 0) c = 1.0f;

	T[0][0] = T[0][0] * a;  T[1][0] = T[1][0] * b; T[2][0] = T[2][0] * c;
	T[0][1] = T[0][1] * a;  T[1][1] = T[1][1] * b; T[2][1] = T[2][1] * c;
	T[0][2] = T[0][2] * a;  T[1][2] = T[1][2] * b; T[2][2] = T[2][2] * c;
	T[0][3] = T[0][3] * a;  T[1][3] = T[1][3] * b; T[2][3] = T[2][3] * c;

	invT[0][0] = invT[0][0] * (1.0f / a); invT[0][1] = invT[0][1] * (1.0f / b); invT[0][2] = invT[0][2] * (1.0f / c);
	invT[1][0] = invT[1][0] * (1.0f / a); invT[1][1] = invT[1][1] * (1.0f / b); invT[1][2] = invT[1][2] * (1.0f / c);
	invT[2][0] = invT[2][0] * (1.0f / a); invT[2][1] = invT[2][1] * (1.0f / b); invT[2][2] = invT[2][2] * (1.0f / c);
	invT[3][0] = invT[3][0] * (1.0f / a); invT[3][1] = invT[3][1] * (1.0f / b); invT[3][2] = invT[3][2] * (1.0f / c);
}

void Instance::hit(Hit &hit){

	hit.transformedRay.origin = invT * (Vector4f(hit.originalRay.origin, 1.0));
	hit.transformedRay.direction = (invT * Vector4f(hit.originalRay.direction, 0.0)).normalize();

	if (m_primitive->bounds && !m_primitive->box.intersect(hit.transformedRay)){

		hit.hitObject = false;
		return;
	}else{

		m_primitive->hit(hit);
		
	}
	
}

bool Instance::shadowHit(Ray &ray, float &hitParameter){

	Ray transformedRay = Ray(invT * (Vector4f(ray.origin, 1.0)), (invT * Vector4f(ray.direction, 0.0)).normalize());

	if (m_primitive->bounds && !m_primitive->box.intersect(transformedRay)){
		
		return false;

	}else{

		return m_primitive->shadowHit(transformedRay, hitParameter);
	}
}

void Instance::setColor(Color color){

	m_color = color;
	m_defaultColor = false;
	
}

Color Instance::getColor(const Vector3f& pos){
	
	// use a texture for the instance
	if (m_useTexture){
		
		if (m_texture){
			
			if (m_texture->getProcedural()){

				return static_cast<ProceduralTexture*>(m_texture.get())->getColor(pos);

			}else{

				std::pair <float, float> uv = getUV(pos);
				return static_cast<ImageTexture*>(m_texture.get())->getTexel(uv.first, uv.second, pos);
			}
		}

		// try to get out the texture color from the primitive
		return m_primitive->getColor(pos);
		
	}else if (!m_defaultColor){
		
		return m_color;

	}else{
		
		//deactivate the texture from the primitive temporary and calculte the color. Set it back after for the next instance
		m_primitive->m_useTexture = false;
		Color color = m_primitive->getColor(pos);
		

		m_primitive->m_useTexture = true;
		
		return	color;
	}


}

std::shared_ptr<Texture> Instance::getTexture(){
	
	if (m_texture){
		
		return m_texture;

	}else{
		
		return m_primitive->getTexture();
	}
}

std::shared_ptr<Material>Instance::getMaterial(){
	
	if (m_material){
		
		return m_material;

	}else{

		return m_primitive->getMaterial();

	}
}

Vector3f Instance::getNormal(const Vector3f& pos){

	return m_primitive->getNormal(pos) * invT;
}

Vector3f Instance::getTangent(const Vector3f& pos){

	return  m_primitive->getTangent(pos) * invT;
}

Vector3f Instance::getBiTangent(const Vector3f& pos){

	return   m_primitive->getBiTangent(pos) * invT;
}

Vector3f Instance::getNormalDu(const Vector3f& pos){

	return  m_primitive->getNormalDu(pos) * invT;
}

Vector3f Instance::getNormalDv(const Vector3f& pos){

	return  m_primitive->getNormalDv(pos) * invT;
}


std::pair<float, float> Instance::getUV(const Vector3f& pos){

	return m_primitive->getUV(pos);
}

BBox &Instance::getBounds(){

	return m_primitive->getBounds();
}

void Instance::calcBounds(){

	m_primitive->calcBounds();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
CompoundedObject::CompoundedObject() : Primitive(){

	m_primitive = NULL;
	m_seperate = false;
	calcBounds();
}

CompoundedObject::~CompoundedObject(){

}

void CompoundedObject::hit(Hit &hit){

	hit.hitObject = false;
	float tmin = hit.t;
	float tminCompoundenObject = hit.t;
	Hit hitCompoundenObject;
	hitCompoundenObject.originalRay = hit.transformedRay;
	
	//after the for loop the hit.transformedRay is transformed to the next local space
	//to avoid transforming to another local space from a following subobject
	//it will be necessary to store the transformation
	Ray ray;
	
	for (unsigned int i = 0; i < m_primitives.size(); i++){

		hitCompoundenObject.transformedRay = hit.transformedRay;
		
		m_primitives[i]->hit(hitCompoundenObject);

		if (hitCompoundenObject.hitObject && hitCompoundenObject.t < tminCompoundenObject) {
			
			m_primitive = m_primitives[i];
			tminCompoundenObject = hitCompoundenObject.t;	
			ray = hitCompoundenObject.transformedRay;
		}	
	}
	
	// find closest primitive
	if (tminCompoundenObject < tmin){

		hit.t = tminCompoundenObject;
		hit.hitObject = true;
		hit.transformedRay = ray;
	}	
}

bool CompoundedObject::shadowHit(Ray &ray, float &hitParameter){

	bool hitObject = false;
	
	for (unsigned int i = 0; i < m_primitives.size(); i++){

		hitObject = hitObject || m_primitives[i]->shadowHit(ray, hitParameter);
		if (hitObject) break;
	}

	return hitObject;
}

Color CompoundedObject::getColor(const Vector3f& pos){
	
	if (m_primitive && m_seperate){
		
		return m_primitive->getColor(pos);

	}else if (m_texture){

		

		if (m_texture->getProcedural()){

			return static_cast<ProceduralTexture*>(m_texture.get())->getColor(pos);

		}else{

			std::pair <float, float> uv = getUV(pos);
			return static_cast<ImageTexture*>(m_texture.get())->getTexel(uv.first, uv.second, pos);
		}

	}else{
		
		return m_color;
	}
}

Vector3f CompoundedObject::getNormal(const Vector3f& pos){

	if (m_primitive){

		return m_primitive->getNormal(pos) ;

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f CompoundedObject::getTangent(const Vector3f& pos){

	if (m_primitive){

		return m_primitive->getTangent(pos);

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f CompoundedObject::getBiTangent(const Vector3f& pos){

	if (m_primitive){

		return m_primitive->getBiTangent(pos);

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f CompoundedObject::getNormalDu(const Vector3f& pos){

	if (m_primitive){

		return m_primitive->getNormalDu(pos);

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}
Vector3f CompoundedObject::getNormalDv(const Vector3f& pos){

	if (m_primitive){

		return m_primitive->getNormalDv(pos);

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

std::pair <float, float> CompoundedObject::getUV(const Vector3f& a_pos){

	if (m_primitive){

		return m_primitive->getUV(a_pos);

	}else{

		float u = 0.0;
		float v = 1.0;

		return std::make_pair(u, v);
	}
}

void CompoundedObject::calcBounds(){

	CompoundedObject::bounds = false;
}



void CompoundedObject::setTextureAll(Texture* texture){

	for (unsigned int i = 0; i < m_primitives.size(); i++){

		m_primitives[i]->setTexture(texture);
	}
}

std::shared_ptr<Texture> CompoundedObject::getTexture(){
	
	if (m_primitive && m_seperate && m_primitive->getTexture()){
		
			return m_primitive->getTexture();

	}else{
		
		return m_texture;
	}
}


void CompoundedObject::setMaterialAll(Material* material){

	for (unsigned int i = 0; i < m_primitives.size(); i++){
		m_primitives[i]->setMaterial(material);
	}
}

void CompoundedObject::setColorAll(const Color& color){

	for (unsigned int i = 0; i < m_primitives.size(); i++){
		m_primitives[i]->setColor(color);
	}
}


std::shared_ptr<Material> CompoundedObject::getMaterial(){
	
	if (m_primitive && m_seperate && m_primitive->getMaterial()){
			return m_primitive->getMaterial();	

	}else{
		
		return m_material;
	}
}


void CompoundedObject::addPrimitive(Primitive* primitive){

	m_primitives.push_back(std::shared_ptr<Primitive>(primitive));
}

void CompoundedObject::setSeperate(bool seperate){

	m_seperate = seperate;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle(const Vector3f &a_V1,
	const Vector3f &a_V2,
	const Vector3f &a_V3) : Primitive(){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	m_cull = false;
	m_smooth = true;
	m_hasNormals = false;
	m_hasTangents = false;
	m_hasNormalDerivatives = false;
	m_edge1 = m_b - m_a;
	m_edge2 = m_c - m_a;

	Vector3f crossProd = Vector3f::cross(m_b - m_a, m_c - m_a);
	abc = crossProd.magnitude();
	m_normal = crossProd.normalize();
	calcBounds();
}

Triangle::Triangle(const Vector3f &a_V1,
	const Vector3f &a_V2,
	const Vector3f &a_V3,
	const bool cull,
	const bool smooth) : Primitive(){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	m_cull = cull;
	m_smooth = smooth;
	m_hasNormals = false;
	m_hasTangents = false;
	m_hasNormalDerivatives = false;
	m_edge1 = m_b - m_a;
	m_edge2 = m_c - m_a;

	Vector3f crossProd = Vector3f::cross(m_b - m_a, m_c - m_a);
	abc = crossProd.magnitude();
	m_normal = crossProd.normalize();
	calcBounds();
}

Triangle::~Triangle(){

}

void Triangle::calcBounds(){

	float delta = 0.001f;
	box.m_pos[0] = min(min(m_a[0], m_b[0]), m_c[0]) - delta;
	box.m_pos[1] = min(min(m_a[1], m_b[1]), m_c[1]) - delta;
	box.m_pos[2] = min(min(m_a[2], m_b[2]), m_c[2]) - delta;

	box.m_size[0] = max(max(m_a[0], m_b[0]), m_c[0]) + delta;
	box.m_size[1] = max(max(m_a[1], m_b[1]), m_c[1]) + delta;
	box.m_size[2] = max(max(m_a[2], m_b[2]), m_c[2]) + delta;

	Triangle::bounds = true;
}

// Möller Trumbore algorithm
void Triangle::hit(Hit &hit){

	//determinat of the triangle
	Vector3f P = Vector3f::cross(hit.transformedRay.direction, m_edge2);
	float det = Vector3f::dot(P, m_edge1);

	if (!m_cull){

		if (fabs(det) < 0.0001f) return;

	}else{

		if (det < 0.0001f) return;
	}

	float inv_det = 1.0f / det;

	// Barycentric Coefficients 
	Vector3f T = hit.transformedRay.origin - Triangle::m_a;

	// the intersection lies outside of the triangle
	float u = Vector3f::dot(T, P) * inv_det;
	if (u < 0.0 || u > 1.0) return;

	// prepare to test v parameter
	Vector3f Q = Vector3f::cross(T, m_edge1);

	// the intersection is outside the triangle
	float v = Vector3f::dot(hit.transformedRay.direction, Q) * inv_det;
	if (v < 0 || u + v > 1) return;

	float result = -1.0;
	result = Vector3f::dot(m_edge2, Q) * inv_det;

	if (result > 0.0){
		
		hit.t = result;
		hit.hitObject = true;
	}
	return;
}

bool Triangle::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}


Color Triangle::getColor(const Vector3f& pos){

	if (m_texture && m_hasTextureCoords){

		Color color;

		if (m_texture->getProcedural()){

			color = static_cast<ProceduralTexture*>(m_texture.get())->getColor(pos);

		}else{

			std::pair <float, float> uv = getUV(pos);
			color = static_cast<ImageTexture*>(m_texture.get())->getTexel(uv.first, uv.second, pos);
		}

		return  color;

	}else{

		return m_color;

	}
}

std::pair <float, float> Triangle::getUV(const Vector3f& a_pos){
	Vector3f apos = m_a - a_pos;
	Vector3f bpos = m_b - a_pos;
	Vector3f cpos = m_c - a_pos;

	//first triangle
	float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;
	//second triangle
	float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;
	//third triangle
	float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

	float u = m_uv1[0] * d1 + m_uv2[0] * d2 + m_uv3[0] * d3;
	float v = m_uv1[1] * d1 + m_uv2[1] * d2 + m_uv3[1] * d3;

	return std::make_pair(u, v);
}

Vector3f Triangle::getNormal(const Vector3f& pos){
	
	if (m_smooth && m_hasNormals){
		
		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;
		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;
		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

		return (m_n1 * d1 + m_n2 * d2 + m_n3 * d3).normalize();

	}
	
	return m_normal;
}

Vector3f Triangle::getTangent(const Vector3f& pos){
	
	if (m_hasTangents){

		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;
		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;
		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;	

		return (m_t1 * d1 + m_t2 * d2 + m_t3 * d3).normalize();

	}

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Triangle::getBiTangent(const Vector3f& pos){

	if (m_hasTangents){

		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;

		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;

		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

		return  (m_bt1 * d1 + m_bt2 * d2 + m_bt3 * d3).normalize();

	}

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Triangle::getNormalDu(const Vector3f& pos){

	if (m_hasNormalDerivatives){
		
		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;

		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;

		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

		/*std::cout << m_nDu1[0] << "  " << m_nDu1[1] << "  " << m_nDu1[2] << std::endl;
		std::cout << m_nDu2[0] << "  " << m_nDu2[1] << "  " << m_nDu2[2] << std::endl;
		std::cout << m_nDu3[0] << "  " << m_nDu3[1] << "  " << m_nDu3[2] << std::endl;
		std::cout << "-------------------------" << std::endl;*/

		return  (m_nDu1 * d1 + m_nDu2 * d2 + m_nDu3 * d3).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

Vector3f Triangle::getNormalDv(const Vector3f& pos){
	
	if (m_hasNormalDerivatives){

		Vector3f apos = Triangle::m_a - pos;
		Vector3f bpos = Triangle::m_b - pos;
		Vector3f cpos = Triangle::m_c - pos;

		//first triangle
		float d1 = Vector3f::cross(bpos, cpos).magnitude() / abc;

		//second triangle
		float d2 = Vector3f::cross(cpos, apos).magnitude() / abc;

		//third triangle
		float d3 = Vector3f::cross(apos, bpos).magnitude() / abc;

		return  (m_nDv1 * d1 + m_nDv2 * d2 + m_nDv3 * d3).normalize();

	}else{

		return Vector3f(0.0, 0.0, 0.0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
Sphere::Sphere() : Primitive(){

	m_centre = Vector3f(0.0, 0.0, 0.0);
	m_sqRadius = 1.0;
	m_radius = 1.0;
	m_invRadius = 1.0f;
	calcBounds();
}

Sphere::Sphere(const Vector3f& a_centre, float a_radius) : Primitive(){

	m_centre = a_centre;
	m_sqRadius = a_radius * a_radius;
	m_radius = a_radius;
	m_invRadius = 1.0f / a_radius;
	calcBounds();
}

Sphere::~Sphere(){

}

void Sphere::hit(Hit &hit) {

	//Use position - origin to get a negative b
	Vector3f L = m_centre - hit.transformedRay.origin;


	float b = Vector3f::dot(L, hit.transformedRay.direction);
	float c = Vector3f::dot(L, L) - m_sqRadius;

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

bool Sphere::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

void Sphere::calcBounds(){
	float delta = 0.00001f;

	box.extend(m_centre - Vector3f(m_radius + delta, m_radius + delta, m_radius + delta));
	box.extend(m_centre + Vector3f(m_radius * 2.0f + delta, m_radius * 2.0f + delta, m_radius * 2.0f + delta));
	Sphere::bounds = true;
}

std::pair <float, float>  Sphere::getUV(const Vector3f& pos){

	//transform the hitPoint to the local system of the sphere (pos - m_centre)
	Vector3f transformedPos = (pos - m_centre) * m_invRadius;

	// map phi from [-pi, pi] to [0, 2pi]  (and rotate the texture 90 degree)
	float phi = atan2(transformedPos[2], transformedPos[0]) + 1.5 * PI;
	// map  phi to  [0, 1]
	float u = 1.0 - (phi * invTWO_PI);

	float theta = acos(transformedPos[1]);
	//theta is in [0, pi] =>  map it to [0, 1]
	float v = 1.0 - theta * invPI;

	return std::make_pair(u, v);
}


Vector3f Sphere::getNormal(const Vector3f& a_pos){

	//N = dp/dphi x dp/dtheta		<==>		N = dp/du x dp/dv

	return  (a_pos - m_centre) * m_invRadius;
}

//x= r* sin(theta) * cos(phi)
//y= r* cos(theta)
//z= r* sin(theta) * sin(phi)

Vector3f Sphere::getTangent(const Vector3f& pos){

	//dp/dphi		<==>		dp/du

	//transform the hitPoint to the local system of the sphere (pos - m_pos)
	Vector3f transformedPos = (pos - m_centre) * m_invRadius;

	float phi = atan2(transformedPos[2], transformedPos[0]) ;
	
	return Vector3f(-sinf(phi), 0.0, cosf(phi));

	//second
	//notice: cosPhi = transformedPos[0] ,	sinPhi = transformedPos[2]
	/*float cosPhi = transformedPos[0];
	float sinPhi = transformedPos[2];

	return Vector3f(-sinPhi, 0.0, cosPhi);*/
}

Vector3f Sphere::getBiTangent(const Vector3f& pos){

	//dp/dtheta		<==>		dp/dv

	//transform the hitPoint to the local system of the sphere (pos - m_centre)
	Vector3f transformedPos = (pos - m_centre) * m_invRadius;

	float theta = acos(transformedPos[1]);
	float phi = atan2(transformedPos[2], transformedPos[0]);
	
	return Vector3f(cosf(theta)*cosf(phi), -sinf(theta), cosf(theta)*sinf(phi)).normalize();

	//second
	//notice: cosPhi = transformedPos[0] ,	sinPhi = transformedPos[2], cosTheta = transformedPos[1]
	/*float cosPhi = transformedPos[0];
	float sinPhi = transformedPos[2];
	float cosTheta = transformedPos[1];

	return Vector3f(transformedPos[1] * transformedPos[0], -sinf(theta), transformedPos[1] * transformedPos[2]);*/
}

Vector3f Sphere::getNormalDu(const Vector3f& pos){

	Vector3f transformedPos = (pos - m_centre) * m_invRadius;

	float n1u = -transformedPos[2];
	float n3u = transformedPos[0];

	return Vector3f(n1u, 0.0, n3u).normalize();
	
}

Vector3f Sphere::getNormalDv(const Vector3f& pos){

	Vector3f transformedPos = (pos - m_centre) * m_invRadius;

	float theta = acos(transformedPos[1]);
	float phi = atan2(transformedPos[2], transformedPos[0]);

	float n1v = transformedPos[1] * cosf(phi);
	float n2v = -sinf(theta);
	float n3v = transformedPos[1] * sinf(phi);

	return Vector3f(n1v, n2v, n3v).normalize();
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::distance = 0.0;
	Plane::m_normal = Vector3f(0.0, 1.0, 0.0);
	calcBounds();
}

Plane::Plane(Vector3f normal, float distance) : Primitive(){

	Plane::distance = distance;
	Plane::m_normal = normal;

	//rotate normal 90 degree arround z-axis
	m_u = Vector3f(normal[1], -normal[0], -normal[2]);
	m_v = Vector3f::cross(normal, m_u);
	calcBounds();
}

Plane::~Plane() {}

void Plane::hit(Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(m_normal, hit.transformedRay.origin)) / Vector3f::dot(m_normal, hit.transformedRay.direction);

	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

bool Plane::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

void Plane::calcBounds(){

	Plane::bounds = false;

}

std::pair <float, float>  Plane::getUV(const Vector3f& a_pos){

	float u = abs(Vector3f::dot(a_pos, m_u));
	float v = abs(Vector3f::dot(a_pos, m_v));

	return std::make_pair(u, v);
}


Vector3f Plane::getNormal(const Vector3f& pos){

	return  m_normal;
}

Vector3f Plane::getTangent(const Vector3f& pos){

	return m_u;
}

Vector3f Plane::getBiTangent(const Vector3f& pos){

	return m_v;
}

Vector3f Plane::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Plane::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Disk::Disk() : Primitive(){

	m_center = Vector3f (0.0, 0.0, 0.0);
	m_normal = Vector3f(0.0, 1.0, 0.0);
	m_radius = 1.0;
	m_sqRadius = 1.0;
	calcBounds();
}

Disk::Disk(Vector3f center, Vector3f normal,  float radius) : Primitive(){

	m_center = center;
	m_radius = radius;
	m_normal = normal;
	m_sqRadius = radius * radius;
	
	calcBounds();
}

Disk::~Disk() {}


void Disk::hit(Hit &hit){

	float result = (Vector3f::dot(m_center - hit.transformedRay.origin, m_normal)) / Vector3f::dot(m_normal, hit.transformedRay.direction);

	if (result <= 0.0001){
		
		hit.hitObject = false;
		return;
	}

	Vector3f p = hit.transformedRay.origin + result * hit.transformedRay.direction;
	Vector3f v = p - m_center;
	
	if (Vector3f::dot(v, v) < m_sqRadius){

		hit.t = result;
		hit.hitObject = true;

		return;
	}

	hit.hitObject = false;
	
}

bool Disk::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

void Disk::calcBounds(){

	Disk::bounds = false;
}

std::pair <float, float>  Disk::getUV(const Vector3f& pos){

	float x =  pos[0];
	float z =  pos[2];

	float len = sqrt(x * x + z * z);

	// rotate the map a quarter round to get it plane with the +z Axis (+ 0.5 PI) 
	float phi = atan2(z,x) + 0.5 * PI;

	float v = len / m_radius;

	// map phi from [-pi, pi] to [0, 1]
	float u = 1.0 - (phi + PI) / TWO_PI;
	return std::make_pair(u, v);
}

Vector3f Disk::getNormal(const Vector3f& pos){

	return  m_normal;
}

Vector3f Disk::getTangent(const Vector3f& pos){

	return Vector3f(0.0 ,0.0, 0.0);
}

Vector3f Disk::getBiTangent(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Disk::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Disk::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Annulus::Annulus() : Primitive(){

	m_center = Vector3f(0.0, -1.0, 0.0);
	m_outerRadius = 1.0;
	m_sqOuterRadius = 1.0;
	m_innerRadius = 0.5;
	m_sqInnerRadius = 0.25;

	calcBounds();
}

Annulus::Annulus(Vector3f center, Vector3f normal, float outerRadius, float innerRadius) : Primitive(){

	m_center = center;
	m_outerRadius = outerRadius;
	m_innerRadius = innerRadius;
	m_normal = normal;
	m_sqOuterRadius = outerRadius * outerRadius;
	m_sqInnerRadius = innerRadius * innerRadius;

	calcBounds();
}

Annulus::~Annulus() {}


void Annulus::hit(Hit &hit){

	float result = (Vector3f::dot(m_center - hit.transformedRay.origin, m_normal)) / Vector3f::dot(m_normal, hit.transformedRay.direction);

	if (result <= 0.0001){
		hit.hitObject = false;
		return;
	}

	Vector3f p = hit.transformedRay.origin + result * hit.transformedRay.direction;
	Vector3f v = p - m_center;
	float dot = Vector3f::dot(v, v);

	if (m_sqInnerRadius <  dot < m_sqOuterRadius){

		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
}

bool Annulus::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

void Annulus::calcBounds(){

	Annulus::bounds = false;
}

std::pair <float, float>  Annulus::getUV(const Vector3f& pos){


	float x = pos[0];
	float z = pos[2];

	float len = sqrt(x * x + z * z);
	float phi = atan2(z, x) + 0.5 * PI;

	
	// v(m_innerRadius) = 0 and  v(m_outerRadius ) = 1
	float v = (len - m_innerRadius) / (m_outerRadius - m_innerRadius);

	// map phi from [-pi, pi] to [0, 1]
	float u = 1.0 - (phi + PI) / TWO_PI;
	return std::make_pair(u, v);
}


Vector3f Annulus::getNormal(const Vector3f& pos){

	return  m_normal;
}

Vector3f Annulus::getTangent(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Annulus::getBiTangent(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f  Annulus::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f  Annulus::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
Torus::Torus() : Primitive(){

	m_radius = 1.0;
	m_tubeRadius = 0.5;
	calcBounds();
}

Torus::Torus(float radius, float tubeRadius) : Primitive(){

	m_radius = radius;
	m_tubeRadius = tubeRadius;
	calcBounds();
}

Torus::~Torus(){

}
// f(x) = (|x|² + a² - b²)² - 4·a²·|xz|² = 0
void Torus::hit(Hit &hit){

	/*if (!box.intersect(_ray)){
		hit.hitObject = false;
		return;
	}*/

	Vector3f ro = hit.transformedRay.origin;
	Vector3f rd = hit.transformedRay.direction;

	double Ra2 = m_radius * m_radius;
	double ra2 = m_tubeRadius * m_tubeRadius;

	double m = Vector3f::dot(ro, ro);
	double n = Vector3f::dot(ro, rd);

	double k = (m - ra2 - Ra2) / 2.0;
	double a = n;
	double b = n*n + Ra2*rd[1] * rd[1] + k;
	double c = k*n + Ra2*ro[1] * rd[1];
	double d = k*k + Ra2*ro[1] * ro[1] - Ra2*ra2;

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
	if (h < 0.0){

		double sQ = sqrt(Q);
		z = 2.0*sQ*cos(acos(R / (sQ*Q)) / 3.0);
	}
	// discriminant < 0
	else{

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

	if (abs(d1)< 0.0001){

		if (d2 < 0.0)  {
			hit.hitObject = false;
			return;
		}
		d2 = sqrt(d2);

	}else{

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
	if (h>0.0){

		h = sqrt(h);

		t1 = -d1 - h - a;
		t2 = -d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {

			result = min(t1, t2);

		}else if (t1 > 0) {

			result = t1;

		}else if (t2 > 0){
			result = t2;
		}
	}


	h = d1o2 - z - d2;
	if (h>0.0){

		h = sqrt(h);
		t1 = d1 - h - a;
		t2 = d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {

			result2 = min(t1, t2);

		}else if (t1 > 0) {

			result2 = t1;

		}else if (t2 > 0){

			result2 = t2;
		}
	}

	if (result2 > 0.0 && result > 0.0){

		result = min(result, result2);

	}else if (result2 > 0.0) result = result2;



	if (result > 0.0){
		
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

bool Torus::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

void Torus::calcBounds(){


	Torus::bounds = false;
	
}

std::pair <float, float>  Torus::getUV(const Vector3f& pos){

	float mainAngle = atan2(pos[2], pos[0]) + 1.5 * PI;
	// map phi from [-pi, pi] to [0, 1]
	float v = 1.0 - (mainAngle / TWO_PI) ;


	// Determine its angle from the x-axis.
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);

	// Now rotate about the x-axis to get the point P into the x-z plane.
	float x = len - m_radius;
	float tubeAngle = atan2(pos[1], x) + PI;
	// map thetafrom [-pi, pi] to [0, 1]
	float u = (tubeAngle / TWO_PI);

	return std::make_pair(v , u );
}


Vector3f Torus::getNormal(const Vector3f& pos){

	// calculate the normal like http://cosinekitty.com/raytrace/chapter13_torus.html
	Vector3f normal;
	float dist = m_radius / sqrtf(pos[0] * pos[0] + pos[2] * pos[2]);

	if (dist > 0.0001){

		normal[0] = pos[0] * (1 - dist);
		normal[1] = pos[1];
		normal[2] = pos[2] * (1 - dist);

	}else{

		normal[0] = pos[0];
		normal[1] = pos[1];
		normal[2] = pos[2];
	}
	

	return normal.normalize();
	
	// calculate the normal with the gradient dp/dx
	/*Vector3f normal;
	float paramSquared = m_radius * m_radius + m_tubeRadius * m_tubeRadius;
	float sumSquared = pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2];
	normal[0] = pos[0] * (sumSquared - paramSquared);
	normal[1] = pos[1] * (sumSquared - paramSquared +  2 * m_radius * m_radius);
	normal[2] = pos[2] * (sumSquared - paramSquared);

	return normal.normalize();*/

	// calculate the normal with derivatives dp/du x dp/dv
	/*float mainAngle = atan2(pos[2], pos[0]) ;
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);

	// Now rotate about the x-axis to get the point P into the x-z plane.
	float x = len - m_radius;
	float tubeAngle = atan2(pos[1], x) ;

	float sinMainSegment = sin(mainAngle);
	float cosMainSegment = cos(mainAngle);

	float sinTubeSegment = sin(tubeAngle);
	float cosTubeSegment = cos(tubeAngle);

	Vector3f normal = Vector3f(cosMainSegment*cosTubeSegment, sinTubeSegment, sinMainSegment*cosTubeSegment);

	return normal;*/
		
}

Vector3f Torus::getTangent(const Vector3f& pos){

	float mainAngle = atan2(pos[2], pos[0]);
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);

	// Now rotate about the x-axis to get the point P into the x-z plane.
	float x = len - m_radius;
	float tubeAngle = atan2(pos[1], x);

	float sinMainSegment = sinf(mainAngle);
	float cosMainSegment = cosf(mainAngle);

	float sinTubeSegment = sinf(tubeAngle);
	float cosTubeSegment = cosf(tubeAngle);

	return Vector3f(-sinTubeSegment *cosMainSegment, cosTubeSegment, -sinTubeSegment *sinMainSegment);
	
}

Vector3f Torus::getBiTangent(const Vector3f& pos){

	float mainAngle = atan2(pos[2], pos[0]);

	Vector3f bitangent = Vector3f(-sinf(mainAngle), 0.0, cosf(mainAngle));

	

	return bitangent;
	
}

Vector3f Torus::getNormalDu(const Vector3f& pos){

	float mainAngle = atan2(pos[2], pos[0]);
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);

	// Now rotate about the x-axis to get the point P into the x-z plane.
	float x = len - m_radius;
	float tubeAngle = atan2(pos[1], x);

	float sinMainSegment = sinf(mainAngle);
	float cosMainSegment = cosf(mainAngle);

	float sinTubeSegment = sinf(tubeAngle);
	float cosTubeSegment = cosf(tubeAngle);

	float tmp = (m_radius + 2 * m_tubeRadius * cosTubeSegment) * m_tubeRadius;
	float n1u = -tmp * cosMainSegment * sinTubeSegment;
	float n2u = tmp * cosTubeSegment - (m_tubeRadius * m_tubeRadius);
	float n3u = -tmp * sinMainSegment * sinTubeSegment;

	return Vector3f(n1u, n2u, n3u).normalize();

}

Vector3f Torus::getNormalDv(const Vector3f& pos){

	float mainAngle = atan2(pos[2], pos[0]);
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);

	// Now rotate about the x-axis to get the point P into the x-z plane.
	float x = len - m_radius;
	float tubeAngle = atan2(pos[1], x);

	float sinMainSegment = sinf(mainAngle);
	float cosMainSegment = cosf(mainAngle);
	float cosTubeSegment = cosf(tubeAngle);

	int sgn = ((m_radius + m_tubeRadius *cosTubeSegment) *cosTubeSegment >= 0) ? 1 : -1;
	Vector3f normalDv = Vector3f(-sinMainSegment * sgn, 0.0, cosMainSegment  * sgn);

	return normalDv;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
Cube::Cube() : Primitive(){

	Cube::m_pos = Vector3f(0.0, 0.0, 0.0);
	Cube::m_size = Vector3f(1.0, 1.0, 1.0);
	calcBounds();
}

Cube::Cube(Vector3f pos, Vector3f size) : Primitive(){

	Cube::m_pos = pos;
	Cube::m_size = size;
	calcBounds();

}

Cube::~Cube(){

}

void Cube::calcBounds(){

	Cube::bounds = false;
}

// http://www.cs.utah.edu/~awilliam/box/ 
void Cube::hit(Hit &hit){

	int sign[3];

	Vector3f invDirection = Vector3f(1.0 / hit.transformedRay.direction[0], 1.0 / hit.transformedRay.direction[2], 1.0 / hit.transformedRay.direction[2]);
	sign[0] = (invDirection[0] < 0);
	sign[1] = (invDirection[1] < 0);
	sign[2] = (invDirection[2] < 0);

	Vector3f bounds[2];
	bounds[0] = m_pos;
	bounds[1] = m_size;

	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	tmin = (bounds[sign[0]][0] - hit.transformedRay.origin[0]) * invDirection[0];

	if (sign[0] < 0){
		side = Sides::Left;	

	}else{
		side = Sides::Right;

	}

	tmax = (bounds[1 - sign[0]][0] - hit.transformedRay.origin[0]) * invDirection[0];
	tymin = (bounds[sign[1]][1] - hit.transformedRay.origin[1]) * invDirection[1];
	tymax = (bounds[1 - sign[1]][1] - hit.transformedRay.origin[1]) * invDirection[1];


	if ((tmin > tymax) || (tymin > tmax)){
		hit.hitObject = false;
		side = Sides::None;
		return;
	}

	if (tymin > tmin){
		tmin = tymin;

		if (sign[1] < 0){
			side = Sides::Bottom;

		}else{
			side = Sides::Top;

		}

		
	}

	if (tymax < tmax){
		tmax = tymax;
	}

	tzmin = (bounds[sign[2]][2] - hit.transformedRay.origin[2]) * invDirection[2];
	tzmax = (bounds[1 - sign[2]][2] - hit.transformedRay.origin[2]) * invDirection[2];
	if ((tmin > tzmax) || (tzmin > tmax)){
		hit.hitObject = false;
		side = Sides::None;
		return;
	}

	if (tzmin > tmin){
		tmin = tzmin;

		if (sign[2] < 0){
			side = Sides::Front;

		}else{

			side = Sides::Back;

		}

	}

	if (tzmax < tmax){
		tmax = tzmax;
	}

	float result = tmin;

	if (result < 0){

		result = tmax;

		if (result < 0) {
			hit.hitObject = false;
			side = Sides::None;
			return;
		}
	}else{

		hit.t = result;
		hit.hitObject = true;
	}

	return;
}

bool Cube::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

Vector3f Cube::getNormal(const Vector3f& a_pos){

	Vector3f centre = (m_pos + m_size) * 0.5;
	Vector3f cp = (a_pos - centre);
	Vector3f tmp = (m_pos - m_size) * 0.5;
	float bias = 1.000001;

	Vector3f normal = Vector3f((float)((int)((cp[0] / fabs(tmp[0])) * bias)),
							   (float)((int)((cp[1] / fabs(tmp[1])) * bias)),
							   (float)((int)((cp[2] / fabs(tmp[2])) * bias)));

	return normal.normalize();
}


Vector3f Cube::getTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}


Vector3f Cube::getBiTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Cube::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Cube::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

std::pair <float, float> Cube::getUV(const Vector3f& a_pos){

	float u = 0.0;
	float v = 1.0;

	return std::make_pair(u, v);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace primitive;
Rectangle::Rectangle() : Primitive(){

	m_pos = Vector3f(-1.0, 0.0, -1.0);
	m_a = Vector3f(0.0, 0.0, 2.0);
	m_b = Vector3f(2.0, 0.0, 0.0);
	m_lenA = 2.0;
	m_lenB = 2.0;
	m_sqA = 4.0;
	m_sqB = 4.0;
	m_area = 4.0;
	m_invArea = 0.25;

	m_normal = Vector3f(0, 1, 0);

	calcBounds();
}

Rectangle::Rectangle(Vector3f pos, Vector3f a, Vector3f b) : Primitive(){

	m_pos = pos;
	m_a = a;
	m_b = b;
	m_lenA = a.magnitude();
	m_lenB = b.magnitude();
	m_sqA = a.sqMagnitude();
	m_sqB = b.sqMagnitude();
	m_area = m_lenA * m_lenB;
	m_invArea = 1.0 / m_area;

	m_normal = Vector3f::cross(a, b).normalize();

	m_v = m_b.normalize();
	m_lenV = m_v.magnitude();

	calcBounds();

}

Rectangle::~Rectangle(){

}

void Rectangle::flipNormal(){

	m_normal = -m_normal;
}

void Rectangle::calcBounds(){

	double delta = 0.0001;

	box.m_pos = m_pos - Vector3f(delta, delta, -delta);
	box.m_size = m_a + m_b + Vector3f(delta, delta, +delta);
	
	Rectangle::bounds = false;
}

void  Rectangle::hit(Hit &hit){
	
	float result = Vector3f::dot(m_pos - hit.transformedRay.origin, m_normal) / Vector3f::dot(hit.transformedRay.direction, m_normal);

	if (result <= 0.0001){
		hit.hitObject = false;
		return;
	}
	Vector3f p = hit.transformedRay.origin + result * hit.transformedRay.direction;
	Vector3f d = p - m_pos;

	float ddota = Vector3f::dot(d, m_a);

	if (ddota < 0.0 || ddota >  m_sqA){
		
		hit.hitObject = false;
		return;
	}

	float ddotb = Vector3f::dot(d, m_b);

	if (ddotb < 0.0 || ddotb > m_sqB){

		hit.hitObject = false;
		return;
	}

	hit.t = result;
	hit.hitObject = true;
}

bool Rectangle::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

Vector3f Rectangle::getNormal(const Vector3f& a_pos){

		return m_normal;	
}

Vector3f Rectangle::getTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}


Vector3f Rectangle::getBiTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Rectangle::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f Rectangle::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

std::pair <float, float> Rectangle::getUV(const Vector3f& pos){

	//transform the hitPoint to the local system of the rectangle (pos - m_pos)
	Vector3f transformedPos = pos - m_pos;

	float v = Vector3f::dot(m_b, transformedPos) * (1.0 / m_sqB);
	float u = Vector3f::dot(m_a, transformedPos) * (1.0 / m_sqA);

	
	return std::make_pair(1.0 - u,  v);

	//return std::make_pair((pos[0] - m_pos[0]) * (1.0 / (m_lenB)), 1.0 - (pos[2] - m_pos[2]) * (1.0 / (m_lenA )));
}

void Rectangle::setSampler(Sampler* sampler){

	m_sampler = std::shared_ptr<Sampler>(sampler);
}

Vector3f Rectangle::sample(){

	Vector2f samplePoint = m_sampler->sampleUnitSquare();

	Vector3f tmp = (m_pos + samplePoint[0] * m_a + samplePoint[1] * m_b);



	/*std::random_device rd1;
	std::default_random_engine generator1(rd1());
	std::uniform_real_distribution<> dis1(0.0, 1.0);

	std::random_device rd2;
	std::default_random_engine generator2(rd2());
	std::uniform_real_distribution<> dis2(0.0, 1.0);

	double r1 = dis1(generator1);
	double r2 = dis2(generator2); 

	Vector3f tmp = m_pos + r1 * m_a + r2 * m_b;*/

	return tmp;
}

float Rectangle::pdf(Hit &hit){
	
	return m_invArea;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
OpenCylinder::OpenCylinder() : Primitive(){

	m_bottom = -1.0;
	m_top = 1.0;
	m_radius = 1.0;
	m_invRadius = 1.0;

	calcBounds();
}

OpenCylinder::OpenCylinder(float bottom, float top, float radius) : Primitive(){

	m_bottom = bottom;
	m_top = top;
	m_radius = radius;
	m_invRadius = 1.0 / radius;

	calcBounds();

}

OpenCylinder::~OpenCylinder(){

}

void OpenCylinder::calcBounds(){

	OpenCylinder::bounds = false;
}

void  OpenCylinder::hit(Hit &hit){

	double ox = hit.transformedRay.origin[0];
	double oy = hit.transformedRay.origin[1];
	double oz = hit.transformedRay.origin[2];
	double dx = hit.transformedRay.direction[0];
	double dy = hit.transformedRay.direction[1];
	double dz = hit.transformedRay.direction[2];

	double a = dx * dx + dz * dz;
	double b = 2.0 * (ox * dx + oz * dz);
	double c = ox * ox + oz * oz - m_radius * m_radius;
	double disc = b * b - 4.0 * a * c;

	double result;

	if (disc < 0.0){

		hit.hitObject = false;
		return;
	}else {
	
		double e = sqrt(disc);
		double denom = 2.0 * a;
		result = (-b - e) / denom;    // smaller root

		if (result >  0.0001) {
			double yhit = oy + result * dy;

			if (yhit > m_bottom && yhit < m_top) {
				hit.t = result;

	
				m_normal = Vector3f( (ox + result * dx) * m_invRadius, 0.0, (oz + result * dz) * m_invRadius);
				// test for hitting from inside
				Vector3f::dot(-hit.transformedRay.direction, m_normal);
				if (Vector3f::dot(hit.transformedRay.direction, m_normal) > 0.0){

					m_normal = -m_normal;
				}

				hit.t = result;
				hit.hitObject = true;
				return;
				
			}
		}

		result = (-b + e) / denom;    // larger root

		if (result > 0.0001) {
			double yhit = oy + result * dy;

			if (yhit > m_bottom && yhit < m_top) {
				hit.t = result;

				m_normal = Vector3f((ox + result * dx) * m_invRadius, 0.0, (oz + result * dz) * m_invRadius);
				// test for hitting inside surface
				if (Vector3f::dot(hit.transformedRay.direction, m_normal) > 0.0){

					m_normal = -m_normal;
				}

				
				hit.t = result;
				hit.hitObject = true;
				return;
			}
		}
	}

	hit.hitObject = false;
}

bool OpenCylinder::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

Vector3f OpenCylinder::getNormal(const Vector3f& a_pos){

		//calaculated at the hit function
		return m_normal;
}


Vector3f OpenCylinder::getTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}


Vector3f OpenCylinder::getBiTangent(const Vector3f& a_pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f OpenCylinder::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f OpenCylinder::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

std::pair <float, float> OpenCylinder::getUV(const Vector3f& pos){

	
	// rotate the map a quarter round to get it plane with the +z Axis (+ 0.5 PI) 
	float phi = atan2(pos[2], pos[0]) + 0.5 * PI;

	// map phi from [-pi, pi] to [0, 1]
	float u = 1.0 - (phi + PI) / TWO_PI;
	float v = (pos[1] - m_bottom) / (m_top - m_bottom);

	return std::make_pair(u, v);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
OpenCone::OpenCone() : Primitive(){

	m_radius = 1.0;
	m_height = 2.0;
	m_center = Vector3f(0.0, 0.0, 0.0);

	m_tanSq = m_radius / m_height;
	m_tanSq = m_tanSq * m_tanSq;
	
	
}

OpenCone::OpenCone(float radius, float height) : Primitive(){

	m_radius = radius;
	m_height = height;
	m_center = Vector3f(0.0, 0.0, 0.0);
}

OpenCone::~OpenCone(){

}

void OpenCone::calcBounds(){

	OpenCone::bounds = false;
}

void OpenCone::hit(Hit &hit){

	double A = hit.transformedRay.origin[0] - m_center[0];
	double B = hit.transformedRay.origin[2] - m_center[2];
	double D = m_height - hit.transformedRay.origin[1] + m_center[1];

	double a = (hit.transformedRay.direction[0] * hit.transformedRay.direction[0]) + (hit.transformedRay.direction[2] * hit.transformedRay.direction[2]) - (m_tanSq*(hit.transformedRay.direction[1] * hit.transformedRay.direction[1]));
	double b = (2 * A*hit.transformedRay.direction[0]) + (2 * B*hit.transformedRay.direction[2]) + (2 * m_tanSq*D*hit.transformedRay.direction[1]);
	double c = (A*A) + (B*B) - (m_tanSq*(D*D));

	double det = b*b - 4.*a*c;

	if (det < 0.0) {
		hit.hitObject = false;
		return;
	}

	det = sqrt(det);
	double t1 = (-b - det) / (2. * a);
	double t2 = (-b + det) / (2. * a);
	double result;

	if (t1 > t2) result = t2;
		else result = t1;

	double r = hit.transformedRay.origin[1] + result * hit.transformedRay.direction[1];

	if ((r > m_center[1]) && (r < m_center[1] + m_height)){
		hit.hitObject = true;
		hit.t = result;
	}

}

bool OpenCone::shadowHit(Ray &ray, float &hitParameter){

	Hit	hitShadow;
	hitShadow.transformedRay = ray;
	hit(hitShadow);
	hitParameter = hitShadow.t;
	return hitShadow.hitObject;
}

Vector3f OpenCone::getNormal(const Vector3f& pos){

	float r = sqrt((pos[0] - m_center[0])*(pos[0] - m_center[0]) + (pos[2] - m_center[2])*(pos[2] - m_center[2]));
	Vector3f n = Vector3f(pos[0] - m_center[0], r*(m_radius / m_height), pos[2] - m_center[2]);
	return n.normalize();
}

Vector3f OpenCone::getTangent(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f OpenCone::getBiTangent(const Vector3f& pos){

	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f OpenCone::getNormalDu(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}

Vector3f OpenCone::getNormalDv(const Vector3f& pos){
	return Vector3f(0.0, 0.0, 0.0);
}


std::pair<float, float> OpenCone::getUV(const Vector3f& pos){

	// [x,y,z] = [ r(z)*cos(phi), r(z)*sin(phi), z]
	// r(z) = m_radius *(1 - z/m_height)
	// r(z) = sqrt(pos[0] * pos[0] + pos[2] * pos[2] ) =: len
	// => len /m_radius = 1 - z/m_height
	// => z = (1 - len / m_radius) * m_height   in [0, m_height]
	// => z = 1 - len / m_radius	in [0, 1]
	// rename v:=z  and u:= phi

	float x = pos[0];
	float z = pos[2];

	float len = sqrt(x * x + z * z );

	float v = 1.0 - len/m_radius;
	//float partRadius = (m_radius / m_height) * (m_height - pos[1]);

	// rotate the map a quarter round to get it plane with the +z Axis (+ 0.5 PI) 
	float phi = atan2(z, x) + 0.5 * PI;
	// map phi from [-pi, pi] to [0, 1]
	float u = 1.0 - (phi + PI) / TWO_PI;

	
	return std::make_pair(u, v);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
SolidCylinder::SolidCylinder() : CompoundedObject(){

	addPrimitive(new Disk(Vector3f(0.0, -1.0, 0.0), Vector3f(0.0, -1.0, 0.0), 1.0)); // bottom
	addPrimitive(new Disk(Vector3f(0.0, 1.0, 0.0), Vector3f(0.0, 1.0, 0.0), 1.0));    // top
	addPrimitive(new OpenCylinder(-1.0, 1.0, 1.0));								   // wall

}

SolidCylinder::SolidCylinder(float bottom, float top, float radius) : CompoundedObject(){

	addPrimitive(new Disk(Vector3f(0.0, bottom, 0.0), Vector3f(0.0, -1.0, 0.0), radius)); // bottom
	addPrimitive(new Disk(Vector3f(0.0, top, 0.0), Vector3f(0.0, 1.0, 0.0), radius));     // top
	addPrimitive(new OpenCylinder(bottom, top, radius));								     // wall
}



SolidCylinder::~SolidCylinder(){

}

void SolidCylinder::hit(Hit &hit){

	if (bounds && !box.intersect(hit.transformedRay)){

		hit.hitObject = false;
		return;

	}else{

		CompoundedObject::hit(hit);
	}
}

bool SolidCylinder::shadowHit(Ray &ray, float &hitParameter){

	return CompoundedObject::shadowHit(ray, hitParameter);
}

void SolidCylinder::setColor(Color color, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setColor(color);  break;
		case Wall: m_primitives[2]->setColor(color);  break;
		case TopDisk: m_primitives[1]->setColor(color);  break;
	}

}

void SolidCylinder::setMaterial(Material* material, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setMaterial(material);  break;
		case Wall: m_primitives[2]->setMaterial(material);  break;
		case TopDisk: m_primitives[1]->setMaterial(material);  break;
	}

}

void SolidCylinder::setTexture(Texture* texture, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setTexture(texture);  break;
		case Wall: m_primitives[2]->setTexture(texture);  break;
		case TopDisk: m_primitives[1]->setTexture(texture);  break;
	}

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
SolidCone::SolidCone() : CompoundedObject(){

	m_radius = 1.0;
	m_top = 1.0;
}

SolidCone::SolidCone(float radius, float top) : CompoundedObject(){

	m_radius = radius;
	m_top = top;
}

SolidCone::~SolidCone(){

}

void SolidCone::hit(Hit &hit){

	if (bounds && !box.intersect(hit.transformedRay)){

		hit.hitObject = false;
		return;
	}else{

		CompoundedObject::hit(hit);
	}
}

bool SolidCone::shadowHit(Ray &ray, float &hitParameter){

	return CompoundedObject::shadowHit(ray, hitParameter);
}

void SolidCone::setColor(Color color, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setColor(color);  break;
		case Wall: m_primitives[1]->setColor(color);  break;
	}

}

void SolidCone::setMaterial(Material* material, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setMaterial(material);  break;
		case Wall: m_primitives[1]->setMaterial(material);  break;
	}

}

void SolidCone::setTexture(Texture* texture, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setTexture(texture);  break;
		case Wall: m_primitives[1]->setTexture(texture);  break;
	}

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
Box::Box() : CompoundedObject(){

	//front
	primitive::Rectangle* rectangle = new primitive::Rectangle(Vector3f(-1.0, -1.0, 1.0), Vector3f(2.0, 0.0, 0.0), Vector3f(0.0, 2.0, 0.0));
	addPrimitive(rectangle);

	//back
	rectangle = new primitive::Rectangle(Vector3f(1.0, -1.0, -1.0), Vector3f(-2.0, 0.0, 0.0), Vector3f(0.0, 2.0, 0.0));
	addPrimitive(rectangle);

	//top
	rectangle = new primitive::Rectangle(Vector3f(-1.0, 1.0, 1.0), Vector3f(2.0, 0.0, 0.0), Vector3f(0.0, 0.0, -2.0));
	addPrimitive(rectangle);
	
	//bottom
	rectangle = new primitive::Rectangle(Vector3f(-1.0, -1.0, -1.0), Vector3f(2.0, 0.0, 0.0), Vector3f(0.0, 0.0, 2.0));
	addPrimitive(rectangle);

	//right
	rectangle = new primitive::Rectangle(Vector3f(1.0, -1.0, 1.0), Vector3f(0.0, 0.0, -2.0), Vector3f(0.0, 2.0, 0.0));
	addPrimitive(rectangle);

	//left
	rectangle = new primitive::Rectangle(Vector3f(-1.0, -1.0, -1.0), Vector3f(0.0, 0.0, 2.0), Vector3f(0.0, 2.0, 0.0));
	addPrimitive(rectangle);

	m_pos = Vector3f(-1.0, -1.0, 1.0);
	m_size = Vector3f(2.0, 2.0, -2.0);
	
}
//left , front, botom
Box::Box(const Vector3f& pos, const Vector3f& size) : CompoundedObject(){


	//front
	primitive::Rectangle* rectangle = new primitive::Rectangle(Vector3f(pos[0], pos[1], pos[2]), Vector3f(size[0], 0.0, 0.0), Vector3f(0.0, size[1], 0.0));
	addPrimitive(rectangle);

	//back
	rectangle = new primitive::Rectangle(Vector3f(pos[0] + size[0], pos[1], pos[2] + size[2]), Vector3f(-size[0], 0.0, 0.0), Vector3f(0.0, size[1], 0.0));
	addPrimitive(rectangle);

	///top
	rectangle = new primitive::Rectangle(Vector3f(pos[0], pos[1] + size[1], pos[2]), Vector3f(size[0], 0.0, 0.0), Vector3f(0.0, 0.0, size[2]));
	addPrimitive(rectangle);

	//bottom
	rectangle = new primitive::Rectangle(Vector3f(pos[0], pos[1], pos[2] + size[2]), Vector3f(size[0], 0.0, 0.0), Vector3f(0.0, 0.0, -size[2]));
	addPrimitive(rectangle);

	//right
	rectangle = new primitive::Rectangle(Vector3f(pos[0] + size[0], pos[1], pos[2]), Vector3f(0.0, 0.0, size[2]), Vector3f(0.0, size[1], 0.0));
	addPrimitive(rectangle);

	//left
	rectangle = new primitive::Rectangle(Vector3f(pos[0], pos[1], pos[2] + size[2]), Vector3f(0.0, 0.0, -size[2]), Vector3f(0.0, size[1], 0.0));
	addPrimitive(rectangle);

	

	m_pos = pos;
	m_size = size;
}

Box::~Box(){

}

void Box::hit(Hit &hit){

	if (bounds && !box.intersect(hit.transformedRay)){

		hit.hitObject = false;
		return;
	}else{

		CompoundedObject::hit(hit);
	}
}

bool Box::shadowHit(Ray &ray, float &hitParameter){

	return CompoundedObject::shadowHit(ray, hitParameter);
}

void Box::setColor(Color color, Components components){

	switch (components){
		case FrontFace: m_primitives[0]->setColor(color);  break;
		case BackFace: m_primitives[1]->setColor(color);  break;
		case TopFace: m_primitives[2]->setColor(color);  break;
		case BottomFace: m_primitives[3]->setColor(color);  break;
		case RightFace: m_primitives[4]->setColor(color);  break;
		case LeftFace: m_primitives[5]->setColor(color);  break;
	}
}

void Box::setMaterial(Material* material, Components components){

	switch (components){
		case FrontFace: m_primitives[0]->setMaterial(material);  break;
		case BackFace: m_primitives[1]->setMaterial(material);  break;
		case TopFace: m_primitives[2]->setMaterial(material);  break;
		case BottomFace: m_primitives[3]->setMaterial(material);  break;
		case RightFace: m_primitives[4]->setMaterial(material);  break;
		case LeftFace: m_primitives[5]->setMaterial(material);  break;
	}
}

void Box::setTexture(Texture* texture, Components components){

	switch (components){
		case FrontFace: m_primitives[0]->setTexture(texture);  break;
		case BackFace: m_primitives[1]->setTexture(texture);  break;
		case TopFace: m_primitives[2]->setTexture(texture);  break;
		case BottomFace: m_primitives[3]->setTexture(texture);  break;
		case RightFace: m_primitives[4]->setTexture(texture);  break;
		case LeftFace: m_primitives[5]->setTexture(texture);  break;
	}
}

void Box::flipNormals(){

	for (unsigned int i = 0; i < m_primitives.size(); i++){
	
		static_cast<primitive::Rectangle*>(m_primitives[i].get())->flipNormal();		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
BeveledCylinder::BeveledCylinder() : CompoundedObject(){

	addPrimitive(new Disk(Vector3f(0.0, -1.0, 0.0), Vector3f(0.0, -1.0, 0.0), 1.0 - 0.2));  // bottom
	addPrimitive(new Disk(Vector3f(0.0, 1.0, 0.0), Vector3f(0.0, 1.0, 0.0), 1.0 - 0.2));    // top
	addPrimitive(new OpenCylinder(-1.0 + 0.2, 1.0 - 0.2, 1.0));							 // wall

	Instance *instance;
	instance = new Instance(new Torus(1.0 - 0.2, 0.2));
	instance->translate(0.0, -1.0 + 0.2, 0);
	addPrimitive(instance);

	instance = new Instance(new Torus(1.0 - 0.2, 0.2));
	instance->translate(0.0, 1.0 - 0.2, 0);
	addPrimitive(instance);
}

BeveledCylinder::BeveledCylinder(float bottom, float top, float radius, float bevelRadius) : CompoundedObject(){

	addPrimitive(new Disk(Vector3f(0.0, bottom, 0.0), Vector3f(0.0, -1.0, 0.0), radius - bevelRadius)); // bottom
	addPrimitive(new Disk(Vector3f(0.0, top, 0.0), Vector3f(0.0, 1.0, 0.0), radius - bevelRadius));     // top
	addPrimitive(new OpenCylinder(bottom + bevelRadius, top - bevelRadius, radius));					 // wall

	Instance *instance;
	Torus *torus;
	torus = new Torus(radius - bevelRadius, bevelRadius);
	torus->setTexture(NULL);

	instance = new Instance(torus);
	instance->translate(0.0, bottom + bevelRadius, 0);
	addPrimitive(instance);

	instance = new Instance(torus);
	instance->translate(0.0, top - bevelRadius, 0);
	addPrimitive(instance);
}

BeveledCylinder::~BeveledCylinder(){

}

void BeveledCylinder::hit(Hit &hit){

	if (bounds && !box.intersect(hit.transformedRay)){

		hit.hitObject = false;
		return;
	}else{

		CompoundedObject::hit(hit);
	}
}

bool BeveledCylinder::shadowHit(Ray &ray, float &hitParameter){

	return CompoundedObject::shadowHit(ray, hitParameter);
}

void BeveledCylinder::setColor(Color color, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setColor(color);  break;
		case BottomTorus: m_primitives[3]->setColor(color); break;
		case Wall: m_primitives[2]->setColor(color);  break;
		case TopTorus: m_primitives[4]->setColor(color); break; 
		case TopDisk: m_primitives[1]->setColor(color);  break;
	}

}

void BeveledCylinder::setMaterial(Material* material, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setMaterial(material);  break;
		case BottomTorus: m_primitives[3]->setMaterial(material); break;
		case Wall: m_primitives[2]->setMaterial(material);  break;
		case TopTorus: m_primitives[4]->setMaterial(material);  break;
		case TopDisk: m_primitives[1]->setMaterial(material);  break;
	}

}

void BeveledCylinder::setTexture(Texture* texture, Components components){

	switch (components){
		case BottomDisk: m_primitives[0]->setTexture(texture);  break;
		case BottomTorus: m_primitives[3]->setTexture(texture); break;
		case Wall: m_primitives[2]->setTexture(texture);  break;
		case TopTorus: m_primitives[4]->setTexture(texture);  break;
		case TopDisk: m_primitives[1]->setTexture(texture);  break;
	}

}