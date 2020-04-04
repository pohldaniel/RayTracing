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
	Camera(const Vector3f &eye,const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);	
	virtual ~Camera();

	void move(float dx, float dy, float dz);
	void rotate(float pitch, float yaw);
	void setPosition(const Vector3f &position);

	const Vector3f &getPosition() const;
	const Vector3f &getCamX() const;
	const Vector3f &getCamY() const;
	const Vector3f &getCamZ() const;
	const Vector3f &getViewDirection() const;

	virtual void renderScene(Scene &scene) = 0;
	virtual void renderScene(Scene &scene, int t1, int t2) = 0;
	void setOffset(const Color& color);

protected:

	void updateView();
	void updateView(const Vector3f &eye, const Vector3f &target, const Vector3f &up);
	void rotateFirstPerson(float pitch, float yaw);

	static const Vector3f WORLD_XAXIS;
	static const Vector3f WORLD_YAXIS;
	static const Vector3f WORLD_ZAXIS;

	Vector3f		INITIAL_XAXIS;
	Vector3f		INITIAL_YAXIS;
	Vector3f		INITIAL_ZAXIS;

	
	Vector3f		m_eye;			
	Vector3f		m_xAxis;	
	Vector3f		m_yAxis;		
	Vector3f		m_zAxis;		
	Vector3f		m_viewDir;		
	
	float			m_accumPitchDegrees;
	std::unique_ptr<Sampler> m_sampler;
	int m_thread_amount = 1;
	Color m_offset;
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Orthographic : public Camera{
public:
	Orthographic();
	Orthographic(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);
	~Orthographic();

	void renderScene(Scene &scene);

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Projection : public Camera {
public:
	Projection();	
	Projection(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);
	~Projection();

	void generateRayDifferential(float _px, float _py, RayDifferential *ray);
	void renderScene(Scene &scene);
	void renderScene(Scene &scene, int t1, int t2);
	void setFovy(float fovy);
	Vector3f rasterToCamera(float _px, float _py);
private:
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
	Pinhole(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler= NULL);
	~Pinhole();

	Vector3f rayDirection(float px, float py) const;
	void renderScene(Scene &scene);
	void renderScene(Scene &scene, int t1, int t2);
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
	FishEye(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);
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
	Spherical(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);
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
	ThinLens(const Vector3f &eye, const Vector3f &target, const Vector3f &up, Sampler *sampler = NULL);
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