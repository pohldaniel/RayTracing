#include "Bitmap.h"
#include "Color.h"

class Texture
{
public:

	Texture(char* path);
	~Texture();

	Color getTexel(float u, float v);
	void setUVScale(float uscale, float vscale);




private:

	int width, height, padWidth;
	Bitmap* bitmap;
	float uscale, vscale;

};

