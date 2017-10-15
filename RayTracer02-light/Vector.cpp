#include "vector.h"



Matrix4f::Matrix4f(){}
Matrix4f::~Matrix4f(){}

const Matrix4f Matrix4f::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

void Matrix4f::rotate(const Vector3f &axis, float degrees)
{
	float rad = (degrees * PI) / 180.0f;

	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float c = cosf(rad);
	float s = sinf(rad);

	mtx[0][0] = (x * x) * (1.0f - c) + c;
	mtx[0][1] = (x * y) * (1.0f - c) + (z * s);
	mtx[0][2] = (x * z) * (1.0f - c) - (y * s);
	mtx[0][3] = 0.0f;

	mtx[1][0] = (y * x) * (1.0f - c) - (z * s);
	mtx[1][1] = (y * y) * (1.0f - c) + c;
	mtx[1][2] = (y * z) * (1.0f - c) + (x * s);
	mtx[1][3] = 0.0f;

	mtx[2][0] = (z * x) * (1.0f - c) + (y * s);
	mtx[2][1] = (z * y) * (1.0f - c) - (x * s);
	mtx[2][2] = (z * z) * (1.0f - c) + c;
	mtx[2][3] = 0.0f;

	mtx[3][0] = 0.0f;
	mtx[3][1] = 0.0f;
	mtx[3][2] = 0.0f;
	mtx[3][3] = 1.0f;
}


Matrix4f::Matrix4f(float m11, float m12, float m13, float m14,
	float m21, float m22, float m23, float m24,
	float m31, float m32, float m33, float m34,
	float m41, float m42, float m43, float m44)
{
	mtx[0][0] = m11, mtx[0][1] = m12, mtx[0][2] = m13, mtx[0][3] = m14;
	mtx[1][0] = m21, mtx[1][1] = m22, mtx[1][2] = m23, mtx[1][3] = m24;
	mtx[2][0] = m31, mtx[2][1] = m32, mtx[2][2] = m33, mtx[2][3] = m34;
	mtx[3][0] = m41, mtx[3][1] = m42, mtx[3][2] = m43, mtx[3][3] = m44;
}

float *Matrix4f::operator[](int row)
{
	return mtx[row];
}

const float *Matrix4f::operator[](int row) const
{
	return mtx[row];
}

//////////////////////////////////////////////////////////////////////

Vector3f::Vector3f(){}
Vector3f::~Vector3f(){}

Vector3f::Vector3f(float x_, float y_, float z_)
{
	x = x_;
	y = y_;
	z = z_;
}

Vector3f &Vector3f::operator-=(const Vector3f &rhs)
{
	x -= rhs.x, y -= rhs.y, z -= rhs.z;
	return *this;
}

Vector3f &Vector3f::operator+=(const Vector3f &rhs)
{
	x += rhs.x, y += rhs.y, z += rhs.z;
	return *this;
}


Vector3f &Vector3f::operator+(const Vector3f &rhs) const{
	Vector3f tmp(*this);
	tmp += rhs;
	return tmp;
}

Vector3f &Vector3f::operator-(const Vector3f &rhs) const
{
	Vector3f tmp(*this);
	tmp -= rhs;
	return tmp;
}

Vector3f Vector3f::operator*(float scalar) const
{
	return Vector3f(x * scalar, y * scalar, z * scalar);
}

Vector3f Vector3f::operator/(float scalar) const{
	
	return Vector3f(x / scalar, y / scalar, z / scalar);
}


Vector3f operator-(const Vector3f &v)
{
	return Vector3f(-v.x, -v.y, -v.z);
}

Vector3f Vector3f::cross(const Vector3f &p, const Vector3f &q)
{
	return Vector3f((p.y * q.z) - (p.z * q.y),
		(p.z * q.x) - (p.x * q.z),
		(p.x * q.y) - (p.y * q.x));
}

float Vector3f::dot(const Vector3f &p, const Vector3f &q)
{
	return (p.x * q.x) + (p.y * q.y) + (p.z * q.z);
}



float Vector3f::magnitude() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}

void Vector3f::normalize(Vector3f &p)
{
	float invMag = 1.0f / p.magnitude();
	p.x *= invMag, p.y *= invMag, p.z *= invMag;
}

Vector3f Vector3f::normalize()
{
	float invMag = 1.0f / magnitude();

	return Vector3f(x*invMag, y*invMag, z*invMag);
}

void Vector3f::set(float x_, float y_, float z_)
{
	x = x_, y = y_, z = z_;
}


Vector3f operator*(const Vector3f &lhs, const Matrix4f &rhs)
{
	return Vector3f((lhs.x * rhs.mtx[0][0]) + (lhs.y * rhs.mtx[1][0]) + (lhs.z * rhs.mtx[2][0]),
		(lhs.x * rhs.mtx[0][1]) + (lhs.y * rhs.mtx[1][1]) + (lhs.z * rhs.mtx[2][1]),
		(lhs.x * rhs.mtx[0][2]) + (lhs.y * rhs.mtx[1][2]) + (lhs.z * rhs.mtx[2][2]));
}





