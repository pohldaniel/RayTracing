#pragma once
#include <memory>
#include "TVector3.h"
#include "SMaterial.h"
#include "SRayHitInfo.h"
#include "Hit.h"
#include "Material.h"
// assumes a,b,c,d are counter clockwise
struct SQuad
{
    SQuad(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d, const SMaterial& material)
        : m_material(material)
        , m_a(a)
        , m_b(b)
        , m_c(c)
        , m_d(d)
    {
		Vector3f e1 = m_b - m_a;
		Vector3f e2 = m_c - m_a;
        m_normal = Vector3f::cross(e1, e2).normalize();
		
    }

	void setMaterial(Material* material2) {
		m_material2 = material2;
	}

	Vector3f    m_a, m_b, m_c, m_d;
    SMaterial   m_material;
	Material* m_material2;
    // calculated!
	Vector3f    m_normal;
};



//=================================================================================
inline bool RayIntersects(const Vector3f& rayPos, const Vector3f& rayDir, const SQuad& quad, SRayHitInfo& info, Hit& hit)
{
    // This function adapted from "Real Time Collision Detection" 5.3.5 Intersecting Line Against Quadrilateral
    // IntersectLineQuad()
	Vector3f pa = quad.m_a - rayPos;
	Vector3f pb = quad.m_b - rayPos;
    Vector3f pc = quad.m_c - rayPos;
    // Determine which triangle to test against by testing against diagonal first
	Vector3f m = Vector3f::cross(pc, rayDir);
	Vector3f r;
    float v = Vector3f::dot(pa, m); // ScalarTriple(pq, pa, pc);
    if (v >= 0.0f) {
        // Test intersection against triangle abc
        float u = -Vector3f::dot(pb, m); // ScalarTriple(pq, pc, pb);
        if (u < 0.0f) return false;	
        float w = Vector3f::dot(Vector3f::cross(rayDir, pb), pa);
        if (w < 0.0f) return false;
        // Compute r, r = u*a + v*b + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.m_a + v*quad.m_b + w*quad.m_c;
    }
    else {
        // Test intersection against triangle dac
		Vector3f pd = quad.m_d - rayPos;
        float u = Vector3f::dot(pd, m); // ScalarTriple(pq, pd, pc);
        if (u < 0.0f) return false;		
        float w = Vector3f::dot(Vector3f::cross(rayDir, pa), pd);
        if (w < 0.0f) return false;
        v = -v;
        // Compute r, r = u*a + v*d + w*c, from barycentric coordinates (u, v, w)
        float denom = 1.0f / (u + v + w);
        u *= denom;
        v *= denom;
        w *= denom; // w = 1.0f - u - v;
        r = u*quad.m_a + v*quad.m_d + w*quad.m_c;
    }

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
	Vector3f normal = quad.m_normal;
    if (Vector3f::dot(quad.m_normal, rayDir) > 0.0f)
        normal = -normal;

    // figure out the time t that we hit the plane (quad)
    float t;
    if (abs(rayDir[0]) > 0.0f)
        t = (r[0] - rayPos[0]) / rayDir[0];
    else if (abs(rayDir[1]) > 0.0f)
        t = (r[1] - rayPos[1]) / rayDir[1];
    else if (abs(rayDir[2]) > 0.0f)
        t = (r[2] - rayPos[2]) / rayDir[2];

    // only positive time hits allowed!
    if (t < 0.0f)
        return false;

    //enforce a max distance if we should
    /*if (info.m_collisionTime >= 0.0 && t > info.m_collisionTime)
        return false;*/

    info.m_collisionTime = t;
    info.m_intersectionPoint = r;
    info.m_material = &quad.m_material;
    info.m_surfaceNormal = normal;

	hit.t = t;
	hit.hitPoint = r;
	hit.normal = normal;
	//hit.material = quad.m_material2;

    return true;
}