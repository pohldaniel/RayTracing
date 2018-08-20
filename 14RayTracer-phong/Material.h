#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <string>

#include "Vector.h"
#include "Color.h"

class Material
{
public:
	Material();
	
	~Material();

	

	std::string colorMapPath;
	std::string bumpMapPath;

	



	Color *m_ambient2;
	Color *m_diffuse2;
	Color *m_specular2;
	int m_shinies;			// neede at the function getSpecular() to determine the surface property

private:

	

	
			
};

#endif