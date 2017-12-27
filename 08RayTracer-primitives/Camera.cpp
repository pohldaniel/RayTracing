#include <cmath>
#include <iostream>

#include "Camera.h"

const Vector3f Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3f Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3f Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera()
{

	m_eye.set(0.0f, 0.0f, 0.0f);
	m_xAxis.set(1.0f, 0.0f, 0.0f);
	m_yAxis.set(0.0f, 1.0f, 0.0f);
	m_zAxis.set(0.0f, 0.0f, 1.0f);
	m_viewDir.set(0.0f, 0.0f, -1.0f);

	m_sampler = new Regular();
	
}


Camera::Camera(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler *sampler){


	m_eye = eye;
	m_xAxis = xAxis;
	m_yAxis = yAxis;
	m_zAxis = zAxis;
	m_sampler = sampler;


	updateView();


}

Camera::Camera(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, const Vector3f &target, const Vector3f &up, Sampler *sampler){

	m_eye = eye;
	m_xAxis = xAxis;
	m_yAxis = yAxis;
	m_zAxis = zAxis;
	m_sampler = sampler;

	updateView(eye, target, up);
}

Camera::Camera(const Camera& c){

	m_eye = c.m_eye;
	m_xAxis = c.m_xAxis;
	m_yAxis = c.m_yAxis;
	m_zAxis = c.m_zAxis;
	m_viewDir = c.m_viewDir;
}



Camera::~Camera()
{
}


void Camera::updateView()
{

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
	
	m_zAxis = m_eye - target;
	Vector3f::normalize(m_zAxis);

	m_xAxis = Vector3f::cross(up, m_zAxis);
	Vector3f::normalize(m_xAxis);

	m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
	Vector3f::normalize(m_yAxis);

	m_viewDir = -m_zAxis;

	// take care of the singularity by hardwiring in specific camera orientations

	if (eye[0] == target[0] && eye[2] == target[2] && eye[1] > target[1]) { // camera looking vertically down
		m_xAxis = Vector3f(0, 0, 1);
		m_yAxis = Vector3f(1, 0, 0);
		m_viewDir = Vector3f(0, 1, 0);
	}

	if (eye[0] == target[0] && eye[2] == target[2] && eye[1] < target[1]) { // camera looking vertically down
		m_xAxis = Vector3f(0, 0, 1);
		m_yAxis = Vector3f(1, 0, 0);
		m_viewDir = Vector3f(0, -1, 0);
	}

}


const Vector3f &Camera::getPosition() const
{
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

Pinhole::Pinhole() :Camera()
{

	m_d = 500;
	m_zoom = 1.0;
}

Pinhole::Pinhole(const Vector3f &eye,
				 const Vector3f &xAxis,
				 const Vector3f &yAxis,
				 const Vector3f &zAxis,
				 const float d,
				 const float zoom,
				 Sampler  *sampler) :Camera(eye, xAxis, yAxis, zAxis, sampler){
	m_d = d;
	m_zoom = zoom;

}

Pinhole::Pinhole(const Vector3f &eye,
				 const Vector3f &xAxis,
				 const Vector3f &yAxis,
				 const Vector3f &zAxis,
				 const Vector3f &target,
				 const Vector3f &up,
				 const float d,
				 const float zoom,
				 Sampler  *sampler) :Camera(eye, xAxis, yAxis, zAxis, target, up, sampler){

	m_d = d;
	m_zoom = zoom;
}



Vector3f & Pinhole::getViewDirection(float px, float py) const{
	Vector3f dir = m_xAxis *px + m_yAxis*py + m_viewDir * m_d;
	Vector3f::normalize(dir);
	
	return(dir);

}

void Pinhole::renderScene(const Scene& scene) {
	
	ViewPlane	vp = scene.vp;
	

	int n = (int)sqrt((float)m_sampler->getNumSamples());
	int numSamples = n*n;

	Color		color;
	Ray			ray;
	Vector2f	sp;
	float		px;
	float		py;
	
	
	vp.s /= m_zoom;
	ray.origin = m_eye;

	for (int y = 0; y < vp.vres; y++){
		for (int x = 0; x < vp.hres; x++){// across 					
			color = Color(0, 0, 0);

			for (int i = 0; i < numSamples; i++){
				sp = m_sampler->sampleUnitSquare();
				px = vp.s * (x - 0.5 * vp.hres + sp[0]);
				py = vp.s * (y - 0.5 * vp.vres + sp[1]);

				

				ray.direction = getViewDirection(px, py);
				color = color + scene.hitObjects(ray).color;

			}
			
			color / numSamples;
			
			scene.setPixel(x, y, color);
		}
	}
}

