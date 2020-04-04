#include <cmath>
#include <iostream>

#include "Camera.h"

const Vector3f Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3f Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3f Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera(){

	m_sampler = std::unique_ptr<Sampler>(new Regular());
	m_offset = Color(0.0, 0.0, 0.0);
	m_accumPitchDegrees = 0.0f;

	m_eye.set(0.0f, 0.0f, 0.0f);
	m_xAxis.set(1.0f, 0.0f, 0.0f);
	m_yAxis.set(0.0f, 1.0f, 0.0f);
	m_zAxis.set(0.0f, 0.0f, 1.0f);
	m_viewDir.set(0.0f, 0.0f, -1.0f);

	INITIAL_XAXIS.set(1.0f, 0.0f, 0.0f);
	INITIAL_YAXIS.set(0.0f, 1.0f, 0.0f);
	INITIAL_ZAXIS.set(0.0f, 0.0f, 1.0f);
}

Camera::Camera(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler  *sampler){
	
	m_sampler = sampler == NULL? std::unique_ptr<Sampler>(new Regular()) : std::unique_ptr<Sampler>(sampler);
	m_offset = Color(0.0, 0.0, 0.0);
	m_accumPitchDegrees = 0.0f;

	m_zAxis = m_eye - target;
	Vector3f::normalize(m_zAxis);
	INITIAL_ZAXIS = m_zAxis;

	m_xAxis = Vector3f::cross(up, m_zAxis);
	Vector3f::normalize(m_xAxis);
	INITIAL_XAXIS = m_xAxis;

	m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
	Vector3f::normalize(m_yAxis);
	INITIAL_YAXIS = m_yAxis;

	updateView(eye, target, up);
}

Camera::~Camera(){}

void Camera::updateView(){

	// Regenerate the camera's local axes to orthogonalize them.
	Vector3f::normalize(m_zAxis);

	m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
	Vector3f::normalize(m_yAxis);

	m_xAxis = Vector3f::cross(m_yAxis, m_zAxis);
	Vector3f::normalize(m_xAxis);

	m_viewDir = -m_zAxis;
}

void Camera::updateView(const Vector3f &eye, const Vector3f &target, const Vector3f &up){

	m_eye = eye;

	m_zAxis = eye - target;
	Vector3f::normalize(m_zAxis);

	m_xAxis = Vector3f::cross(up, m_zAxis);
	Vector3f::normalize(m_xAxis);

	m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
	Vector3f::normalize(m_yAxis);

	m_viewDir = -m_zAxis;

	// take care of the singularity by hardwiring in specific camera orientations
	/*if (eye[0] == target[0] && eye[2] == target[2] && eye[1] > target[1]) { // camera looking vertically down
		m_xAxis = Vector3f(-1.0, 0.0, 0.0);
		m_yAxis = Vector3f(0.0, 0.0, -1.0);
		m_viewDir = Vector3f(0.0, -1.0, 0.0);
	}

	if (eye[0] == target[0] && eye[2] == target[2] && eye[1] < target[1]) { // camera looking vertically up
		m_xAxis = Vector3f(-1.0, 0.0, 0.0);
		m_yAxis = Vector3f(0.0, 0.0, -1.0);
		m_viewDir = Vector3f(0.0, 1.0, 0.0);	
	}*/
}

void Camera::move(float dx, float dy, float dz){
	m_eye += m_xAxis * dx;
	m_eye += INITIAL_YAXIS * dy;
	m_eye += m_viewDir * dz;
}


void Camera::rotate(float yaw, float pitch){
	// Rotates the camera
	rotateFirstPerson(pitch, yaw);
	updateView();
}

void Camera::rotateFirstPerson(float pitch, float yaw){
	m_accumPitchDegrees += pitch;

	if (m_accumPitchDegrees > 90.0f){
		pitch = 90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = 90.0f;
	}

	if (m_accumPitchDegrees < -90.0f){
		pitch = -90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = -90.0f;
	}

	Matrix4f rotMtx;
	// Rotate camera's existing x and z axes about the world y axis.
	if (yaw != 0.0f){
		rotMtx.rotate(WORLD_YAXIS, yaw);
		m_xAxis = m_xAxis * rotMtx;
		m_zAxis = m_zAxis * rotMtx;
	}

	// Rotate camera's existing y and z axes about its existing x axis.
	if (pitch != 0.0f){
		rotMtx.rotate(m_xAxis, pitch);
		m_yAxis = m_yAxis * rotMtx;
		m_zAxis = m_zAxis * rotMtx;
	}
}

void Camera::setOffset(const Color& color){
	m_offset = color;
}

const Vector3f &Camera::getPosition() const{
	return m_eye;
}

const Vector3f &Camera::getCamX() const{
	return m_xAxis;
}

const Vector3f &Camera::getCamY() const{
	return m_yAxis;
}

const Vector3f &Camera::getCamZ() const{

	return m_zAxis;
}
const Vector3f &Camera::getViewDirection() const{
	return m_viewDir;
}

///////////////////////////////////////////////////////////////////////
Orthographic::~Orthographic(){}

Orthographic::Orthographic() :Camera(){};

Orthographic::Orthographic(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler) : Camera(eye, target, up, sampler){	}

void Orthographic::renderScene(Scene& scene) {
	std::cout << "Render scene!" << std::endl;
	ViewPlane	vp = scene.getViewPlane();

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	Color		color;
	Ray			ray;
	Vector2f	sp;
	
	ray.direction = Vector3f(0.0, 0.0, -1.0);

	for (int y = 0; y < vp.vres; y++){
		for (int x = 0; x < vp.hres; x++){
			color = Color(0, 0, 0);

			for (int i = 0; i < numSamples; i++){
				sp = m_sampler->sampleUnitSquare();
				ray.origin = Vector3f((float)(x - 0.5 * vp.hres + sp[0]),(float)( y - 0.5 * vp.vres + sp[0]), getPosition()[2])*vp.s;
				color = color + scene.hitObjects(ray).color;
			}

			color = color / numSamples;

			scene.setPixel(x, y, color + m_offset);
		}
	}
}
///////////////////////////////////////////////////////////////////////
Projection::~Projection(){}

Projection::Projection() :Camera(){

	m_fovy = 40.0;

	m_hres = 640;
	m_vres = 480;
	m_aspectRatio = (float)m_hres / m_vres;
	m_scale = (float)tan((PI / 360) * m_fovy);

	m_dxCamera = rasterToCamera(1.0, 0.0) - rasterToCamera(0.0, 0.0);
	m_dyCamera = rasterToCamera(0.0, 1.0) - rasterToCamera(0.0, 0.0);
}

Projection::Projection(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler  *sampler) : Camera(eye, target, up, sampler){

	m_fovy = 40.0;

	m_hres = 512;
	m_vres = 512;
	m_aspectRatio = (float)m_hres / m_vres;
	m_scale = (float)tan((PI / 360) * m_fovy);

	m_dxCamera = rasterToCamera(1.0, 0.0) - rasterToCamera(0.0, 0.0);
	m_dyCamera = rasterToCamera(0.0, 1.0) - rasterToCamera(0.0, 0.0);

}

Vector3f Projection::rasterToCamera(float px, float py){

	float u = px / m_hres;
	float v = py / m_vres;

	return (m_viewDir + m_xAxis *(m_aspectRatio*(2 * u - 1))*m_scale + m_yAxis *(2 * v - 1)*m_scale).normalize();
}

void Projection::setFovy(float fovy){

	m_fovy = fovy;
}

void Projection::generateRayDifferential(float _px, float _py, RayDifferential *rayDiff){

	Vector3f direction = rasterToCamera(_px, _py);

	rayDiff->m_rxOrigin = rayDiff->m_ryOrigin = rayDiff->origin;
	rayDiff->m_rxDirection = (direction + m_dxCamera).normalize();
	rayDiff->m_ryDirection = (direction + m_dyCamera).normalize();
}

void Projection::renderScene(Scene& scene) {

	for (int i = 0; i < m_thread_amount; i++) {
		for (int j = 0; j < m_thread_amount; j++) {
			renderScene(scene, i, j);
		}
	}
}

void Projection::renderScene(Scene& scene, int t1, int t2) {

	ViewPlane	vp = scene.getViewPlane();

	Color		color;
	Ray			ray;
	Vector2f	sp;

	float		px;
	float		py;

	float aspectRatio = (float)vp.hres / vp.vres;
	float scale = (float)tan((PI / 360) * m_fovy);

	int width = vp.hres;
	int height = vp.vres;
	int width_portion = width / m_thread_amount;
	int height_portion = height / m_thread_amount;

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	ray.origin = m_eye;

	for (int y = height_portion * t2; y < (height_portion * t2) + height_portion; y++) {

		for (int x = width_portion * t1; x < (width_portion * t1) + width_portion; x++) {


			color = Color(0, 0, 0);

			for (int i = 0; i < numSamples; i++) {

				sp = m_sampler->sampleUnitSquare();

				px = (x + sp[0]);
				py = (y + sp[1]);

				ray.direction = rasterToCamera(px, py).normalize();

				color = color + scene.hitObjects(ray).color;
			}

			color = color / numSamples;
			scene.setPixel(x, y, color + m_offset);
		}
	}

}

///////////////////////////////////////////////////////////////////////
Pinhole::~Pinhole(){}

Pinhole::Pinhole() :Camera(){

	m_zoom = 1.0;
	m_d = 500;
}

Pinhole::Pinhole(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler  *sampler) : Camera(eye, target, up, sampler){

	m_zoom = 1.0;
	m_d = 500;
}


void Pinhole::setZoom(float zoom){
	m_zoom = zoom;
}
void Pinhole::setViewPlaneDistance(float distance){
	m_d = distance;
}

Vector3f  Pinhole::rayDirection(float px, float py) const{
	Vector3f dir = (m_xAxis *px + m_yAxis*py + m_viewDir * m_d).normalize();

	return(dir);

}

void Pinhole::renderScene(Scene& scene) {

	for (int i = 0; i < m_thread_amount; i++) {
		for (int j = 0; j < m_thread_amount; j++) {
			renderScene(scene, i, j);
		}
	}
}

void Pinhole::renderScene(Scene& scene, int t1, int t2) {

	ViewPlane	vp = scene.getViewPlane();

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	Color		color;
	Ray			ray;
	Vector2f	sp;
	float		px;
	float		py;

	int width = vp.hres;
	int height = vp.vres;
	int width_portion = width / m_thread_amount;
	int height_portion = height / m_thread_amount;

	vp.s /= m_zoom;
	ray.origin = m_eye;

	for (int y = height_portion * t2; y < (height_portion * t2) + height_portion; y++) {
		for (int x = width_portion * t1; x < (width_portion * t1) + width_portion; x++) {
		
			color = Color(0, 0, 0);
			for (int i = 0; i < numSamples; i++) {

				sp = m_sampler->sampleUnitSquare();

				px = (x - 0.5 * width + sp[0]);
				py = (y - 0.5 * height + sp[1]);

				ray.direction = rayDirection(px, py);

				color = color + scene.hitObjects(ray).color;
			}

			color = color / numSamples;
			scene.setPixel(x, y, color + m_offset);
		}
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////
FishEye::~FishEye(){}

FishEye::FishEye() :Camera(){

	m_psiMax = 180.0;
}

FishEye::FishEye(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler) : Camera(eye, target, up, sampler){

	m_psiMax = 180.0;
}

void FishEye::setFov(const float fov){

	m_psiMax = fov / 2.0;
}

Vector3f FishEye::rayDirection(float px, float py, const int hres, const int vres, const float s, float& r_squared) const{

	float pnx = 2.0 / (s * hres) * px;
	float pny = 2.0 / (s * vres) * py;

	r_squared = pnx * pnx + pny * pny;

	if (r_squared <= 1.0) {
		float r = sqrt(r_squared);
		float psi = r * m_psiMax * PI_ON_180;
		float sinPsi = sin(psi);
		float cosPsi = cos(psi);
		float sinAlpha = pny / r;
		float cosAlpha = pnx / r;

		return sinPsi * cosAlpha * m_xAxis + sinPsi * sinAlpha * m_yAxis + cosPsi * m_viewDir;

	}else{
		return Vector3f(0.0, 0.0, 0.0);
	}
}

void FishEye::renderScene(Scene &scene) {

	std::cout << "Render scene!" << std::endl;
	ViewPlane	vp = scene.getViewPlane();

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;
	
	Color color;
	Ray	ray;
	Vector2f	sp;
	float		px;
	float		py;
	float		rSquared;				// sum of squares of normalised device coordinat

	int width = vp.hres;
	int height = vp.vres;

	ray.origin = m_eye;

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			color = Color(0, 0, 0);

			for (int i = 0; i < numSamples; i++){
				sp = m_sampler->sampleUnitSquare();
				px = vp.s * (x - 0.5f * vp.hres + sp[0]);
				py = vp.s * (y - 0.5f * vp.vres + sp[1]);

				ray.direction = rayDirection(px, py, width, height, vp.s, rSquared);

				if (rSquared <= 1.0){
					color = color + scene.hitObjects(ray).color;
				}
			}

			color = color / numSamples;
			scene.setPixel(x, y, color + m_offset);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////
Spherical::~Spherical(){}

Spherical::Spherical() :Camera(){

	m_psiMax = 90.0;
	m_lambdaMax = 180.0;
}

Spherical::Spherical(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler) : Camera(eye, target, up, sampler){

	m_psiMax = 90.0;
	m_lambdaMax = 180.0;
}

void Spherical::setHorizontalFov(const float fov){

	m_psiMax = fov / 4.0;
}

void Spherical::setVerticalFov(const float fov){

	m_lambdaMax = fov ;
}


Vector3f Spherical::rayDirection(float px, float py, const int hres, const int vres, const float s) const{

	//compute the normalised device coordinates
	float pnx = 2.0 / (s * hres) * px;
	float pny = 2.0 / (s * vres) * py;

	//compute the angles lambda and phi in radians
	float lambda = pnx * m_lambdaMax * PI_ON_180;
	float psi = pny * m_psiMax * PI_ON_180;

	//compute the regular azimuth and polar angles
	float phi = lambda -  PI ;
	float theta = 0.5 * PI - psi;

	float sinPhi = sin(phi);
	float cosPhi = cos(phi);
	float sinTheta = sin(theta);
	float cosTheta = cos(theta);

	return sinTheta * sinPhi * m_xAxis + cosTheta * m_yAxis + sinTheta * cosPhi * m_viewDir;
}

void Spherical::renderScene(Scene &scene) {

	std::cout << "Render scene!" << std::endl;
	ViewPlane	vp = scene.getViewPlane();

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	Color color;
	Ray	ray;
	Vector2f	sp;
	float		px;
	float		py;

	int width = vp.hres;
	int height = vp.vres;

	ray.origin = m_eye;

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			color = Color(0.5, 0, 0);

			for (int i = 0; i < numSamples; i++){
				sp = m_sampler->sampleUnitSquare();
				px = vp.s * (x - 0.5f * vp.hres + sp[0]);
				py = vp.s * (y - 0.5f * vp.vres + sp[1]);

				ray.direction = rayDirection(px, py, width, height, vp.s);

				color = color + scene.hitObjects(ray).color;
				
			}

			color = color / numSamples;
			scene.setPixel(x, y, color + m_offset);
		}
	}
}
///////////////////////////////////////////////////////////////////////
ThinLens::~ThinLens(){}

ThinLens::ThinLens() :Camera(){

	m_zoom = 1.0;
	m_d = 500;
}

ThinLens::ThinLens(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler) : Camera(eye, target, up, sampler){
	m_zoom = 1.0;
	m_d = 500;
}


void ThinLens::setZoom(float zoom){
	m_zoom = zoom;
}
void ThinLens::setViewPlaneDistance(float distance){
	m_d = distance;
}
void ThinLens::setFocalDistance(float f){
	m_f = f;
}
void ThinLens::setLensRadius(float lensRadius){
	m_lensRadius = lensRadius;
}

Vector3f ThinLens::rayDirection(float px, float py, float lx, float ly) const{

	float _px = px* (m_f / m_d); float _py = py* (m_f / m_d);
	Vector3f dir = (_px - lx) * m_xAxis + (_py - ly) * m_yAxis + m_f * m_viewDir;
	return dir.normalize();
}

void ThinLens::renderScene(Scene &scene){

	std::cout << "Render scene!" << std::endl;
	ViewPlane	vp = scene.getViewPlane();

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	Color color;
	Ray	ray;
	Vector2f	sp;
	Vector2f	dp;
	Vector2f	lp;
	float		px;
	float		py;
	
	int width = vp.hres;
	int height = vp.vres;

	m_sampler->mapSamplesToUnitDisk();

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			color = Color(0.0, 0.0, 0.0);

			for (int i = 0; i < numSamples; i++){
				sp = m_sampler->sampleUnitSquare();
				px = vp.s * (x - 0.5f * vp.hres + sp[0]);
				py = vp.s * (y - 0.5f * vp.vres + sp[1]);

				dp = m_sampler->sampleUnitDisk();
				lp = dp * m_lensRadius;
				ray.origin = m_eye + lp[0]*m_xAxis + lp[1] * m_yAxis;
				ray.direction = rayDirection(px, py, lp[0], lp[1]);
				color = color + scene.hitObjects(ray).color;
			}

			color = color / numSamples;
			scene.setPixel(x, y, color + m_offset);
		}
	}
}