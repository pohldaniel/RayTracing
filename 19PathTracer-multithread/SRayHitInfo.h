#pragma once

#include "TVector3.h"
#include "Vector.h"

struct SMaterial;

struct SRayHitInfo
{
    SRayHitInfo()
        : m_material(nullptr)
        , m_collisionTime(-1.0f)
    { }

    const SMaterial*    m_material;
	Vector3f            m_intersectionPoint;
	Vector3f            m_surfaceNormal;
    float               m_collisionTime;
};