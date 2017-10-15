#ifndef _CAMERA_H
#define _CAMERA_H

#include "Vector.h"

class Camera
{


public:



	Camera();
	Camera(Vector3f &eye, Vector3f &xAxis, Vector3f &yAxis, Vector3f &zAxis);
	~Camera();


	void move(float dx, float dy, float dz);
	void rotate(float pitch, float yaw);


	const Vector3f &getPosition() const;
	void setPosition(const Vector3f &position);

	const Vector3f &getCamX() const;
	const Vector3f &getCamY() const;
	const Vector3f &getCamZ() const;
	const Vector3f &getViewDirection() const;


private:


	void rotateFirstPerson(float pitch, float yaw);
	void updateView();


	static const Vector3f WORLD_XAXIS;
	static const Vector3f WORLD_YAXIS;
	static const Vector3f WORLD_ZAXIS;


	float			m_accumPitchDegrees;
	Vector3f		m_eye;
	Vector3f		m_xAxis;
	Vector3f		m_yAxis;
	Vector3f		m_zAxis;
	Vector3f		m_viewDir;


};

class ProjectionMap
{
public:
	Vector3f &map(int x, int y, const  Vector3f &viewdir, const Vector3f &camright, const Vector3f &camup);
	ProjectionMap(int height, int width, float fovy);
	~ProjectionMap();

private:
	int height;
	int width;
	float scale;
	float aspectRatio;
};



#endif // _CAMERA_H