#include <cmath>
#include <iostream>

#include "camera.h"

const Vector3f Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3f Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3f Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera()
{

	m_eye.set(0.0f, 0.0f, 0.0f);
	m_xAxis.set(1.0f, 0.0f, 0.0f);
	m_yAxis.set(0.0f, 1.0f, 0.0f);
	m_zAxis.set(0.0f, 0.0f, 1.0f);

	
}


Camera::Camera(Vector3f &eye, Vector3f &xAxis, Vector3f &yAxis, Vector3f &zAxis)
{


	m_eye = eye;
	m_xAxis = xAxis;
	m_yAxis = yAxis;
	m_zAxis = zAxis;

	updateView();


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


void Camera::move(float dx, float dy, float dz)
{
	// Moves the camera by dx world units to the left or right; dy
	// world units upwards or downwards; and dz world units forwards
	// or backwards.

	m_eye += m_xAxis * dx;
	m_eye += WORLD_YAXIS * dy;
	m_eye += m_viewDir * dz;

}


void Camera::rotate(float yaw, float pitch)
{
	// Rotates the camera

	rotateFirstPerson(pitch, yaw);
	updateView();
}

void Camera::rotateFirstPerson(float pitch, float yaw)
{
	m_accumPitchDegrees += pitch;

	if (m_accumPitchDegrees > 90.0f)
	{
		pitch = 90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = 90.0f;
	}

	if (m_accumPitchDegrees < -90.0f)
	{
		pitch = -90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = -90.0f;
	}

	Matrix4f rotMtx;

	// Rotate camera's existing x and z axes about the world y axis.
	if (yaw != 0.0f)
	{
		rotMtx.rotate(WORLD_YAXIS, yaw);
		m_xAxis = m_xAxis * rotMtx;
		m_zAxis = m_zAxis * rotMtx;
	}

	// Rotate camera's existing y and z axes about its existing x axis.
	if (pitch != 0.0f)
	{
		rotMtx.rotate(m_xAxis, pitch);
		m_yAxis = m_yAxis * rotMtx;
		m_zAxis = m_zAxis * rotMtx;
	}
}



void Camera::setPosition(const Vector3f &position)
{
	m_eye = position;

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


//////////////////////////////////////////////////////////////////////

ProjectionMap::ProjectionMap(int height, int width, float fovy){

	ProjectionMap::width = width;
	ProjectionMap::height = height;

	ProjectionMap::aspectRatio = (float)width / height;
	ProjectionMap::scale = tan((PI / 360) * fovy);
}


Vector3f &ProjectionMap::map(int x, int y, const  Vector3f &viewdir, const Vector3f &camright, const Vector3f &camup){

	float xamnt = (float)(x + 0.5)/width;
	float yamnt = (float)(y + 0.5) / height;
	
	// Step 1: Map from Raster Space to Camera Space without perspektive projection
	// Vector3f tmp = viewdir - camright *(aspectRatio*(2 * xamnt - 1)) - camup *(2 * yamnt - 1) 

	// Step 2: Introduce the fovy/perspektive projetion ---> scale = tan((PI / 360) * fovy) 
	// Vector3f tmp = viewdir - camright *(aspectRatio*(2 * xamnt - 1)) * scale - camup *(2 * yamnt - 1) * scale 

	return (viewdir + camright *(aspectRatio*(2 * xamnt - 1))*scale+ camup *(2 * yamnt - 1)*scale).normalize();
}


Vector3f &ProjectionMap::map2(int x, int y, int q, int p, int n, const  Vector3f &viewdir, const Vector3f &camright, const Vector3f &camup){

	float xamnt = (float)(x + (q + 0.5)/n)/width;
	float yamnt = (float)(y + (p + 0.5)/n)/height;
	
	
	// Step 1: Map from Raster Space to Camera Space without perspektive projection
	// Vector3f tmp = viewdir - camright *(aspectRatio*(2 * xamnt - 1)) - camup *(2 * yamnt - 1) 

	// Step 2: Introduce the fovy/perspektive projetion ---> scale = tan((PI / 360) * fovy) 
	// Vector3f tmp = viewdir - camright *(aspectRatio*(2 * xamnt - 1)) * scale - camup *(2 * yamnt - 1) * scale 

	return (viewdir + camright *(aspectRatio*(2 * xamnt - 1))*scale + camup *(2 * yamnt - 1)*scale).normalize();
}


ProjectionMap::~ProjectionMap(){
}