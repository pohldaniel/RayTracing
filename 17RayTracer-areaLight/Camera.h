#ifndef _CAMERA_H
#define _CAMERA_H

#include "Vector.h"
#include "Color.h"
#include "Ray.h"
#include "Scene.h"
#include "Sampler.h"


class Scene;
class Camera{

public:

	Camera();
	Camera(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler  *sampler);
	Camera(const Vector3f &eye,const Vector3f &target, const Vector3f &up, Sampler *sampler);	
	virtual ~Camera();

	const Vector3f &getPosition() const;
	const Vector3f &getCamX() const;
	const Vector3f &getCamY() const;
	const Vector3f &getCamZ() const;
	const Vector3f &getViewDirection() const;

	virtual void renderScene(Scene &scene) = 0;
	void setOffset(const Color& color);

protected:

	void updateView();
	void updateView(const Vector3f &eye, const Vector3f &target, const Vector3f &up);

	static const Vector3f WORLD_XAXIS;
	static const Vector3f WORLD_YAXIS;
	static const Vector3f WORLD_ZAXIS;

	Vector3f		m_eye;			
	Vector3f		m_xAxis;	
	Vector3f		m_yAxis;		
	Vector3f		m_zAxis;		
	Vector3f		m_viewDir;		
	
	std::unique_ptr<Sampler> m_sampler;

	Color m_offset;
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Orthographic : public Camera{
public:
	Orthographic();

	Orthographic(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		Sampler *sampler);

	Orthographic(const Vector3f &eye,
		const Vector3f &target,
		const Vector3f &up,
		Sampler *sampler);

	~Orthographic();

	void renderScene(Scene &scene);

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Projection : public Camera {
public:
	Projection();
	~Projection();

	Projection(const Vector3f &eye,
		const Vector3f &xAxis,
		const Vector3f &yAxis,
		const Vector3f &zAxis,
		Sampler *sampler);

	Projection(const Vector3f &eye,
		const Vector3f &target,
		const Vector3f &up,
		Sampler *sampler);

	void generateRayDifferential(float _px, float _py, RayDifferential *ray);

	void renderScene(Scene &scene);
	void setFovy(float fovy);

private:

	Vector3f rasterToCamera(float _px, float _py);

	Vector3f m_dxCamera, m_dyCamera;
	float m_fovy;

	int m_hres;
	int m_vres;
	float m_aspectRatio;
	float m_scale;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Pinhole : public Camera{
public:

	Pinhole();
	Pinhole(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler *sampler);
	Pinhole(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler);
	~Pinhole();

	Vector3f rayDirection(float px, float py) const;
	void renderScene(Scene &scene);

	void setZoom(float zoom);
	void setViewPlaneDistance(float distance);

private:
	float	m_zoom;		// zoom factor
	float	m_d;		// view plane distance
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class FishEye : public Camera {
public:

	FishEye();
	FishEye(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler *sampler);
	FishEye(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler);
	~FishEye();

	Vector3f rayDirection(float px, float py, const int hres, const int vres, const float s, float& r_squared) const;
	void renderScene(Scene &scene);

	void setFov(const float fov);

private:

	float	m_psiMax;	// in degrees
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Spherical : public Camera {
public:

	Spherical();
	Spherical(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler *sampler);
	Spherical(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler);
	~Spherical();

	Vector3f rayDirection(float px, float py, const int hres, const int vres, const float s) const;
	void renderScene(Scene &scene);

	void setHorizontalFov(const float fov);
	void setVerticalFov(const float fov);

private:

	float m_psiMax;	// in degrees
	float m_lambdaMax;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class ThinLens : public Camera {

public:

	ThinLens();
	ThinLens(const Vector3f &eye, const Vector3f &xAxis, const Vector3f &yAxis, const Vector3f &zAxis, Sampler *sampler);
	ThinLens(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler);
	~ThinLens();

	Vector3f rayDirection(float px, float py, float lx, float ly) const;
	void renderScene(Scene &scene);
	
	void setZoom(float zoom);
	void setViewPlaneDistance(float distance);
	void setFocalDistance(float f);
	void setLensRadius(float lensRadius);
private:

	float	m_zoom;			// zoom factor
	float	m_d;			// view plane distance
	float	m_f;			// focal distance
	float	m_lensRadius;	// lens radius
};

#endif // _CAMERA_H