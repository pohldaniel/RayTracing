#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <memory>

#include "Vector.h"
#include "Bitmap.h"
#include "Color.h"

class Mapping{

public:

	Mapping();
	~Mapping();

	virtual std::pair<float, float> getUV(const Vector3f& pos) = 0;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class SphericalMap : public Mapping {

public:
	SphericalMap();
	~SphericalMap();

	std::pair<float, float> getUV(const Vector3f& pos);

	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class LightProbe : public Mapping {

public:
	typedef enum { lightProbe, panoramic } MapType;

	LightProbe();
	~LightProbe();

	std::pair<float, float> getUV(const Vector3f& pos);

	void setMapType(MapType mapType);

private:

	MapType m_mapType;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class RectangularMap : public Mapping {

public:
	
	RectangularMap(const Vector3f& pos, const Vector3f& a, const Vector3f& b);
	~RectangularMap();

	std::pair<float, float> getUV(const Vector3f& pos);

private:

	Vector3f		m_pos, m_a, m_b;   		// attributes needed for texturing a non generic object
	float			m_sqA, m_sqB;			//

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class DiskMap : public Mapping {

public:

	DiskMap(const Vector3f& pos, const float outerRadius);
	DiskMap(const Vector3f& pos, const float outerRadius, const float innerRadius);
	DiskMap(const float outerRadius);
	DiskMap(const float outerRadius, const float innerRadius);
	~DiskMap();

	std::pair<float, float> getUV(const Vector3f& pos);

private:

	float m_innerRadius, m_outerRadius;	// attributes needed for texturing a non generic object
	Vector3f m_pos;

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Texture{

public:
	
	Texture();
	virtual ~Texture();

	bool getProcedural();
	void setMapping(Mapping* mapping);

protected:
	
	std::unique_ptr<Mapping> m_mapping;
	bool m_procedural;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class ImageTexture : public Texture{

public:
	ImageTexture();
	ImageTexture(const char* path);
	virtual ~ImageTexture();

	Color getTexel(const float u, const float v, const Vector3f& pos);
	Color getSmoothTexel(const float a_u, const float a_v);

	void setUVScale(const float uscale, const float vscale);
	

protected:
	int m_width, m_height, m_padWidth;
	std::unique_ptr<Bitmap> m_bitmap;
	

private:

	
	float m_uscale, m_vscale;
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class BlurTexture : public ImageTexture{

public:
	BlurTexture(double sigma, int filterheight, int filterwidth);
	BlurTexture(double sigma, int filterheight, int filterwidth, const char* path);
	~BlurTexture();

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class ProceduralTexture : public Texture{

public:
	ProceduralTexture();
	virtual ~ProceduralTexture();

	virtual Color getColor(const Vector3f& pos) = 0;

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class CheckerTexture : public ProceduralTexture{

public:
	CheckerTexture();
	virtual ~CheckerTexture();


	void setColor1(const float r, const float g, const float b);
	void setColor1(const float c);
	void setColor1(const Color& c);
	void setColor2(const float r, const float g, const float b);
	void setColor2(const float c);
	void setColor2(const Color& c);
	void setLineColor(const float r, const float g, const float b);
	void setLineColor(const float c);
	void setLineColor(const Color& c);

protected:

	Color	m_color1;							// checker color 1
	Color	m_color2;							// checker color 2
	Color	m_lineColor;						// line color	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Checker3D : public CheckerTexture {

public:

	Checker3D();
	~Checker3D();

	Color getColor(const Vector3f& pos);


	void setSize(const float size);
	void setOutlineWidth(const float width);

private:

	float	m_outlineWidth;
	float	m_size;

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class PlaneChecker : public CheckerTexture {

public:

	PlaneChecker();
	~PlaneChecker();

	Color getColor(const Vector3f& pos);

	void setOutlineWidth(const float width);
	void setSize(const float size);

private:

	float	m_outlineWidth;
	float	m_size;

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class SphereChecker : public CheckerTexture {

public:

	SphereChecker();
	~SphereChecker();

	Color getColor(const Vector3f& pos);

	void setNumHorizontalCheckers(const int numHorizontal);
	void setNumVerticalCheckers(const int numVertical);
	void setHorizontalLineWidth(const float width);
	void setVerticalLineWidth(const float width);
	
private:

	int		m_numHorizontalCheckers;			// number of checkers in the horizontal (azithum) direction
	int		m_numVerticalCheckers;			// number of checkers in the vertical (polar) direction
	float	m_horizontalLineWidth;			// width of the horizontal lines as a fraction of the checker width
	float	m_verticalLineWidth;				// width of the vertical lines as a fraction of the checker width
	
	
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CylinderChecker : public CheckerTexture {

public:

	CylinderChecker();
	~CylinderChecker();

	Color getColor(const Vector3f& pos);

	void setNumHorizontalCheckers(const int numHorizontal);
	void setNumVerticalCheckers(const int numVertical);
	void setHorizontalLineWidth(const float width);
	void setVerticalLineWidth(const float width);
	
private:

	int		m_numHorizontalCheckers;			// number of checkers in the horizontal (azithum) direction
	int		m_numVerticalCheckers;			// number of checkers in the vertical (polar) direction
	float	m_horizontalLineWidth;			// width of the horizontal lines as a fraction of the checker width
	float	m_verticalLineWidth;				// width of the vertical lines as a fraction of the checker width

};
//////////////////////////////////////////////////////////////////////////////////////////////////
class DiskChecker : public CheckerTexture {

public:

	DiskChecker();
	~DiskChecker();

	Color getColor(const Vector3f& pos);

	void setNumAngularCheckers(const int numAngular);
	void setNumRadialCheckers(const int numRadial);
	void setAngularLineWidth(const float width);
	void setRadialLineWidth(const float width);

private:

	int		m_numAngularCheckers;			
	int		m_numRadialCheckers;			
	float	m_angularLineWidth;			
	float	m_radialLineWidth;				
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class ConeChecker : public CheckerTexture {

public:

	ConeChecker();
	~ConeChecker();

	Color getColor(const Vector3f& pos);

	void setNumHorizontalCheckers(const int numHorizontal);
	void setNumVerticalCheckers(const int numVertical);
	void setHorizontalLineWidth(const float width);
	void setVerticalLineWidth(const float width);
	
private:

	int		m_numHorizontalCheckers;			// number of checkers in the horizontal (azithum) direction
	int		m_numVerticalCheckers;			// number of checkers in the vertical (polar) direction
	float	m_horizontalLineWidth;			// width of the horizontal lines as a fraction of the checker width
	float	m_verticalLineWidth;				// width of the vertical lines as a fraction of the checker width
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class RectangleChecker : public CheckerTexture {

public:

	RectangleChecker();
	~RectangleChecker();

	Color getColor(const Vector3f& pos);

	void setNumXCheckers(const int numX);
	void setNumZCheckers(const int numZ);
	void setXLineWidth(const float width);
	void setZLineWidth(const float width);
	

private:

	int		m_numXCheckers;			// number of checkers in the horizontal (azithum) direction
	int		m_numZCheckers;			// number of checkers in the vertical (polar) direction
	float	m_XLineWidth;			// width of the horizontal lines as a fraction of the checker width
	float	m_ZLineWidth;			// width of the vertical lines as a fraction of the checker width

	

	
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class TorusChecker : public CheckerTexture {

public:

	TorusChecker();
	~TorusChecker();

	Color getColor(const Vector3f& pos);

	void setNumHorizontalCheckers(const int numHorizontal);
	void setNumVerticalCheckers(const int numVertical);
	void setHorizontalLineWidth(const float width);
	void setVerticalLineWidth(const float width);
	
	void setRadius(float radius);

private:

	int		m_numHorizontalCheckers;			// number of checkers in the horizontal (azithum) direction
	int		m_numVerticalCheckers;			// number of checkers in the vertical (polar) direction
	float	m_horizontalLineWidth;			// width of the horizontal lines as a fraction of the checker width
	float	m_verticalLineWidth;				// width of the vertical lines as a fraction of the checker width
	float   m_radius;
	
};
#endif
