#include <cmath>
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

void Camera::perspective(float fovx, float aspect, float znear, float zfar){

	// Construct a projection matrix based on the horizontal field of view
	// 'fovx' rather than the more traditional vertical field of view 'fovy'.

	float e = tanf(PI*fovx / 360);
	float xScale = (1 / e) / aspect;
	float yScale = 1 / e;



	m_projMatrix[0][0] = xScale;
	m_projMatrix[0][1] = 0.0f;
	m_projMatrix[0][2] = 0.0f;
	m_projMatrix[0][3] = 0.0f;

	m_projMatrix[1][0] = 0.0f;
	m_projMatrix[1][1] = yScale;
	m_projMatrix[1][2] = 0.0f;
	m_projMatrix[1][3] = 0.0f;

	m_projMatrix[2][0] = 0.0f;
	m_projMatrix[2][1] = 0.0f;
	m_projMatrix[2][2] = (zfar + znear) / (znear - zfar);
	m_projMatrix[2][3] = -1.0f;

	m_projMatrix[3][0] = 0.0f;
	m_projMatrix[3][1] = 0.0f;
	m_projMatrix[3][2] = (2.0f * zfar * znear) / (znear - zfar);
	m_projMatrix[3][3] = 0.0f;

	m_fovx = fovx;
	m_aspectRatio = aspect;
	m_znear = znear;
	m_zfar = zfar;
}


void Camera::orthographic(float left, float right, float bottom, float top, float znear, float zfar){

	m_orthMatrix[0][0] = 2 / (right - left);
	m_orthMatrix[0][1] = 0.0f;
	m_orthMatrix[0][2] = 0.0f;
	m_orthMatrix[0][3] = 0.0f;

	m_orthMatrix[1][0] = 0.0f;
	m_orthMatrix[1][1] = 2 / (top - bottom);
	m_orthMatrix[1][2] = 0.0f;
	m_orthMatrix[1][3] = 0.0f;

	m_orthMatrix[2][0] = 0.0f;
	m_orthMatrix[2][1] = 0.0f;
	m_orthMatrix[2][2] = 2 / (znear - zfar);
	m_orthMatrix[2][3] = 0.0f;

	m_orthMatrix[3][0] = (right + left) / (left - right);
	m_orthMatrix[3][1] = (top + bottom) / (bottom - top);
	m_orthMatrix[3][2] = (zfar + znear) / (znear - zfar);
	m_orthMatrix[3][3] = 1.0f;

	m_znear = znear;
	m_zfar = zfar;
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



const Vector3f &Camera::getViewDirection() const{
	return m_viewDir;
}

const Matrix4f &Camera::getProjectionMatrix() const{

	return m_projMatrix;
}

const Matrix4f &Camera::getOrthographicMatrix() const{

	return m_orthMatrix;
}