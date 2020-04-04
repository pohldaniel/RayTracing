#ifndef _TVECTOR3_H
#define _TVECTOR3_H

#include <array>
#include <stdint.h>
#include <array>

typedef uint8_t uint8;
typedef std::array<uint8, 3> TPixelBGRU8;
typedef std::array<float, 3> TPixelRGBF32;
typedef std::array<float, 3> TVector3;

const float c_pi = 3.14159265359f;

//=================================================================================
inline float Clamp(float v, float min, float max) {

	if (v < min)
		return min;
	else if (v > max)
		return max;
	else
		return v;
}

//=================================================================================
// from 0 to 1
float RandomFloat() {

	/*
	// Xorshift random number algorithm invented by George Marsaglia
	static uint32_t rng_state = 0xf2eec0de;
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return float(rng_state) * (1.0f / 4294967296.0f);
	*/

	// alternately, using a standard c++ prng
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(mt);
}

//=================================================================================
float RandomFloat(float min, float max) {

	return min + (max - min) * RandomFloat();
}



//=================================================================================
// Vector operations
inline float LengthSq (const TVector3& v)
{
    return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
}

inline float Length (const TVector3& v)
{
    return sqrt(LengthSq(v));
}

inline TVector3 Normalize (const TVector3& v)
{
    float len = Length(v);
    return
    {
        v[0] / len,
        v[1] / len,
        v[2] / len
    };
}

inline TVector3 operator+ (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2]
    };
}

inline TVector3 operator+ (const TVector3& a, float b)
{
    return
    {
        a[0] + b,
        a[1] + b,
        a[2] + b
    };
}

inline TVector3 operator- (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    };
}

inline void operator-= (TVector3& a, const TVector3& b)
{
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
}

inline void operator+= (TVector3& a, const TVector3& b)
{
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}

inline TVector3 operator* (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] * b[0],
        a[1] * b[1],
        a[2] * b[2]
    };
}

inline TVector3 operator* (float b, const TVector3& a)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator* (const TVector3& a, float b)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator/ (const TVector3& a, float b)
{
    return
    {
        a[0] / b,
        a[1] / b,
        a[2] / b
    };
}

inline void operator*= (TVector3& a, float b)
{
    a[0] *= b;
    a[1] *= b;
    a[2] *= b;
}

inline TVector3 operator- (const TVector3& a)
{
    return
    {
        -a[0], -a[1], -a[2]
    };
}

inline float Dot (const TVector3& a, const TVector3& b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

inline TVector3 Cross (const TVector3& a, const TVector3& b)
{
    return
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

inline float ScalarTriple (const TVector3& a, const TVector3& b, const TVector3& c)
{
    return Dot(Cross(a, b), c);
}

//=================================================================================
inline TVector3 CosineSampleHemisphere (const TVector3& normal)
{
    // from smallpt: http://www.kevinbeason.com/smallpt/

    float r1 = 2.0f * c_pi *RandomFloat();
    float r2 = RandomFloat();
    float r2s = sqrt(r2);

    TVector3 w = normal;
    TVector3 u;
    if (fabs(w[0]) > 0.1f)
        u = Cross({ 0.0f, 1.0f, 0.0f }, w);
    else
        u = Cross({ 1.0f, 0.0f, 0.0f }, w);

    u = Normalize(u);
    TVector3 v = Cross(w, u);
    TVector3 d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));
    d = Normalize(d);

    return d;
}

//=================================================================================
inline TVector3 UniformSampleHemisphere (const TVector3& N)
{
    // Uniform point on sphere
    // from http://mathworld.wolfram.com/SpherePointPicking.html
    float u = RandomFloat();
    float v = RandomFloat();

    float theta = 2.0f * c_pi * u;
    float phi = acos(2.0f * v - 1.0f);

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    TVector3 dir;
    dir[0] = cosTheta * sinPhi;
    dir[1] = sinTheta * sinPhi;
    dir[2] = cosPhi;

    // if our vector is facing the wrong way vs the normal, flip it!
    if (Dot(dir, N) <= 0.0f)
        dir *= -1.0f;

    return dir;
}

//=================================================================================
inline TVector3 ChangeBasis (const TVector3& v, const TVector3& xAxis, const TVector3& yAxis, const TVector3& zAxis)
{
    return 
    {
        Dot(v, { xAxis[0], yAxis[0], zAxis[0]}),
        Dot(v, { xAxis[1], yAxis[1], zAxis[1]}),
        Dot(v, { xAxis[2], yAxis[2], zAxis[2]})
    };
}

//=================================================================================
inline TVector3 UndoChangeBasis (const TVector3& v, const TVector3& xAxis, const TVector3& yAxis, const TVector3& zAxis)
{
    return
    {
        Dot(v, xAxis),
        Dot(v, yAxis),
        Dot(v, zAxis)
    };
}

#endif // _STIMER_H