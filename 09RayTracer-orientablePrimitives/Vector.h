#ifndef _VECTOR_H
#define _VECTOR_H

#if !defined(MATHLIB_H)
#define MATHLIB_H

#include <cmath>

//-----------------------------------------------------------------------------
// Common math functions and constants.
//-----------------------------------------------------------------------------
const float PI = 3.1415926f;

class Vector2f{
public:
	Vector2f();
	Vector2f(float x_, float y_);
	~Vector2f();

	Vector2f operator*(float scalar) const;

	float &operator[](int index);
	const float operator[](int index) const;

private:

	float vec[2];

};



//-----------------------------------------------------------------------------
// A 3-component vector class that represents a row vector.
//-----------------------------------------------------------------------------

class Vector3f
{

	friend Vector3f operator-(const Vector3f &v);

public:

	Vector3f();
	Vector3f(float x_, float y_, float z_);
	~Vector3f();


	static Vector3f cross(const Vector3f &p, const Vector3f &q);
	static float dot(const Vector3f &p, const Vector3f &q);
	static void normalize(Vector3f &p);

	Vector3f normalize();
	float magnitude() const;

	void set(float x_, float y_, float z_);



	float &operator[](int index);
	const float operator[](int index) const;

	const float* getVec()const;

	Vector3f &operator+=(const Vector3f &rhs);
	Vector3f &operator-=(const Vector3f &rhs);

	Vector3f &operator+(const Vector3f &rhs) const;
	Vector3f &operator-(const Vector3f &rhs) const;

	Vector3f operator*(float scalar) const;
	Vector3f operator/(float scalar) const;

private:
	float vec[3];
};


class Vector4f{
public:
	Vector4f();
	Vector4f(float x_, float y_, float z_, float w_);
	Vector4f(const Vector3f &rhs);
	~Vector4f();

	float &operator[](int index);
	const float operator[](int index) const;

private:

	float vec[4];
};

//-----------------------------------------------------------------------------
// A homogeneous row-major 4x4 matrix class.
//
// Multiplies Vector3s to the left of the matrix.
//-----------------------------------------------------------------------------

class Matrix4f
{
	friend Vector3f operator*(const Vector4f &lhs, const Matrix4f &rhs);
	friend Vector3f operator*(const Vector3f &lhs, const Matrix4f &rhs);
	friend Matrix4f operator*(float scalar, const Matrix4f &rhs);

public:


	static const Matrix4f IDENTITY;

	Matrix4f();
	Matrix4f(float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44);
	~Matrix4f();

	float *operator[](int row);
	const float *operator[](int row) const;
	Matrix4f &operator*=(const Matrix4f &rhs);
	Matrix4f operator*(const Matrix4f &rhs) const;

	void identity();
	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float c);

private:
	float mtx[4][4];
};

#endif
#endif