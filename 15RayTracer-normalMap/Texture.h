#include <memory>

#include "Bitmap.h"
#include "Color.h"

class Texture
{
public:
	

	Texture(const char* path);
	~Texture();

	Color getTexel(const float u, const float v);
	void setUVScale(const float uscale, const float vscale);




private:

	int width, height, padWidth;
	std::unique_ptr<Bitmap> bitmap;
	float uscale, vscale;
	
};

