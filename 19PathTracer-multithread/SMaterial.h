#ifndef _SMATERIAL_H
#define _SMATERIAL_H

#include "Path.h"

struct SMaterial{

    SMaterial(const Vector3f& emissive, const Vector3f& diffuse)
        : m_emissive(emissive)
        , m_diffuse(diffuse)
    { }
	Vector3f    m_emissive;
	Vector3f    m_diffuse;
};

#endif 