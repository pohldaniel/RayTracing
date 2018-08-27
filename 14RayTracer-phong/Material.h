#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <string>

#include "Vector.h"
#include "Color.h"

class Material
{
public:
	Material();
	Material(const Color &ambient, const Color &diffuse, const Color &specular, const int shinies);



	~Material();

	

	Color m_ambient;
	Color m_diffuse;
	Color m_specular;
	int m_shinies;			
	
	std::string colorMapPath;
	std::string bumpMapPath;
	
	// neede at the function getSpecular() to determine the surface property

private:

	

	
			
};

#endif