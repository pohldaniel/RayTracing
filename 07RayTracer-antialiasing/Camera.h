#ifndef _CAMERA_H
#define _CAMERA_H

#include "Vector.h"
#include "Color.h"
#include "Ray.h"
#include "Scene.h"
#include "Sampler.h"


class Scene;
class Camera
{


public:

	Camera();
	Camera(const Vector3f &eye,
		   const Vector3f &xAxis,
		   const Vector3f &yAxis,
		   const Vector3f &zAxis,
		   const float zoom,
		   Sampler  *sampler);

	Camera(const Vector3f &eye,
		   const Vector3f &xAxis,
		   const Vector3f &yAxis,
		   const Vector3f &zAxis,
		   const Vector3f &target,
		   const Vector3f &up,
		   const float zoom,
		   Sampler  *sampler);

	Camera(const Camera& camera);	// copy constructor

	~Camera();




	const Vector3f &getPosition() const;
	const Vector3f &getCamX() const;
	const Vector3f &getCamY() const;
	const Vector3f &getCamZ() const;
	const Vector3f &getViewDirection() const;


	virtual void renderScene(const Scene& scene) = 0;

protected:


	void updateView();
	void updateView(const Vector3f &eye, const Vector3f &target, const Vector3f &up);

	static const Vector3f WORLD_XAXIS;
	static const Vector3f WORLD_YAXIS;
	static const Vector3f WORLD_ZAXIS;


	Vector3f		m_eye;			// eye
	Vector3f		m_xAxis;		// u
	Vector3f		m_yAxis;		// v
	Vector3f		m_zAxis;		// w --> eye - target
	Vector3f		m_viewDir;		// 
	float			m_zoom;			// zoom factor

	Sampler	*m_sampler;

};

class Orthographic : public Camera{
public:
	Orthographic();

	Orthographic(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		const float zoom,
		Sampler *sampler);

	Orthographic(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		const Vector3f &target,
		const Vector3f &up,
		const float zoom,
		Sampler *sampler);

	~Orthographic();

	void renderScene(const Scene& scene);

};

class Pinhole : public Camera{
public:

	Pinhole();

	Pinhole(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		const float zoom,
		const float d,
		Sampler *sampler);


	Pinhole(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		const Vector3f &target,
		const Vector3f &up,
		const float zoom,
		const float d,
		Sampler  *sampler);

	~Pinhole();

	Vector3f &getViewDirection(float px, float py) const;
	void renderScene(const Scene& scene);

private:

	float	m_d;		// view plane distance
};



#endif // _CAMERA_H