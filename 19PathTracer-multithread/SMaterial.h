#ifndef _SMATERIAL_H
#define _SMATERIAL_H



struct SMaterial{

    SMaterial(const TPixelRGBF32& emissive, const TPixelRGBF32& diffuse)
        : m_emissive(emissive)
        , m_diffuse(diffuse)
    { }
    TPixelRGBF32    m_emissive;
    TPixelRGBF32    m_diffuse;
};

#endif 