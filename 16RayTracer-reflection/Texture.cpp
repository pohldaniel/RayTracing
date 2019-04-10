#include <windows.h>
#include <atlstr.h> 
#include <iostream>
#include "Texture.h"
#include "Primitive.h"

Mapping::Mapping(){

}

Mapping::~Mapping(){

}
///////////////////////////////////////////////////////////////////////////////////////////////
SphericalMap::SphericalMap(){}

SphericalMap::~SphericalMap(){}

std::pair<float, float> SphericalMap::getUV(const Vector3f& pos){

	// map phi from [-pi, pi] to [0, 1]
	float phi = atan2(pos[2], pos[0]) + 0.5 * PI;
	// next, map theta and phi to (u, v) in [0, 1] X [0, 1]
	float u = 1.0 - (phi + PI) / TWO_PI;

	float theta = acos(pos[1]);
	float v = 1.0 - theta * invPI;

	return std::make_pair(u, v);
}
///////////////////////////////////////////////////////////////////////////////////////////////
LightProbe::LightProbe(){

	m_mapType = lightProbe;
}

LightProbe::~LightProbe(){}

std::pair<float, float> LightProbe::getUV(const Vector3f& pos){

	float x = pos[0];
	float y = pos[1];
	float z = pos[2];

	float d = sqrt(x * x + y * y);
	float sinBeta = y / d;
	float cosBeta = x / d;
	float alpha;

	if (m_mapType == lightProbe)   // the default
		alpha = acos(z);

	if (m_mapType == panoramic)
		alpha = acos(-z);

	float r = alpha * invPI;
	float u = 1.0 - (1.0 + r * cosBeta) * 0.5;
	float v =  (1.0 + r * sinBeta) * 0.5;

	return std::make_pair(u, v);
}

void LightProbe::setMapType(MapType mapType){

	m_mapType = mapType;
}
///////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(){

}

Texture::~Texture(){

}

bool Texture::getProcedural(){

	return m_procedural;
}
///////////////////////////////////////////////////////////////////////////////////////////////
ImageTexture::ImageTexture(){

	m_bitmap = std::unique_ptr<Bitmap>(new Bitmap());
	m_bitmap->createNullBitmap(200);

	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;
	m_uscale = 1.0;
	m_vscale = 1.0;

	m_procedural = false;
	m_mapping = NULL;
}

ImageTexture::ImageTexture(const char* path){

	m_bitmap = std::unique_ptr<Bitmap>(new Bitmap());

	if (!m_bitmap->loadBitmap24(path)){
		std::cout << "create nulltexture" << std::endl;
		m_bitmap->createNullBitmap(200);
	}


	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;
	m_uscale = 1.0;
	m_vscale = 1.0;

	m_procedural = false;
	m_mapping = NULL;
}

ImageTexture::~ImageTexture(){


}

void ImageTexture::setUVScale(const float uscale, const float vscale){

	m_uscale = uscale;
	m_vscale = vscale;
}

void ImageTexture::setMapping(Mapping* mapping){

	m_mapping = std::unique_ptr<Mapping>(mapping);

}

Color ImageTexture::getTexel(const float a_u, const float a_v, const Vector3f& pos){

	float _u;
	float _v;

	if (m_mapping){

		std::pair <float, float> uv = m_mapping->getUV(pos);
		_u = uv.first;
		_v = uv.second;

	}else{

		_u = a_u;
		_v = a_v;
	}

	int u = m_width  * m_uscale * _u;
	int v = m_height * m_vscale * _v;

	u = (m_width + u % m_width) % m_width;
	v = (m_height + v % m_height) % m_height;


	int r = m_bitmap->data[m_padWidth*v + 3 * u];
	int g = m_bitmap->data[m_padWidth*v + 3 * u + 1];
	int b = m_bitmap->data[m_padWidth*v + 3 * u + 2];


	return Color(r / 255.0, g / 255.0, b / 255.0);
}

Color  ImageTexture::getSmoothTexel(const float a_u, const float a_v){

	float u = m_width  * m_uscale * a_u;
	float v = m_height * m_vscale * a_v;

	int u1 = (m_width + (int)u % m_width) % m_width;
	int v1 = (m_height + (int)v % m_height) % m_height;
	int u2 = (u1 + 1) % m_width;
	int v2 = (v1 + 1) % m_height;

	// calculate fractional parts of u and v
	float fracu = u - floorf(u);
	float fracv = v - floorf(v);
	// calculate weight factors
	float w1 = (1 - fracu) * (1 - fracv);
	float w2 = fracu * (1 - fracv);
	float w3 = (1 - fracu) * fracv;
	float w4 = fracu *  fracv;

	// fetch four texels
	Color c1 = Color(m_bitmap->data[m_padWidth*v1 + 3 * u1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u1 + 1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u1 + 2] / 255.0);
	Color c2 = Color(m_bitmap->data[m_padWidth*v1 + 3 * u2] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u2 + 1] / 255.0, m_bitmap->data[m_padWidth*v1 + 3 * u2 + 2] / 255.0);
	Color c3 = Color(m_bitmap->data[m_padWidth*v2 + 3 * u1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u1 + 1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u1 + 2] / 255.0);
	Color c4 = Color(m_bitmap->data[m_padWidth*v2 + 3 * u2] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u2 + 1] / 255.0, m_bitmap->data[m_padWidth*v2 + 3 * u2 + 2] / 255.0);

	// scale and sum the four colors
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}

///////////////////////////////////////////////////////////////////////////////////////////////
BlurTexture::BlurTexture(double a_sigma, int a_filterheight, int a_filterwidth) : ImageTexture(){

	GaussianBlur *gaussianBlur = new GaussianBlur(a_sigma, a_filterheight, a_filterwidth);
	gaussianBlur->createKernel();
	gaussianBlur->applyFilter(this->m_bitmap.get());

	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;

	delete gaussianBlur;
}

BlurTexture::BlurTexture(double a_sigma, int a_filterheight, int a_filterwidth, const char* path) : ImageTexture(path){

	GaussianBlur *gaussianBlur = new GaussianBlur(a_sigma, a_filterheight, a_filterwidth);
	gaussianBlur->createKernel();
	gaussianBlur->applyFilter(this->m_bitmap.get());

	m_width = m_bitmap->width;
	m_height = m_bitmap->height;
	m_padWidth = m_bitmap->padWidth;

	delete gaussianBlur;
}

BlurTexture::~BlurTexture(){

}
///////////////////////////////////////////////////////////////////////////////////////////////
ProceduralTexture::ProceduralTexture(){
	m_procedural = true;
}

ProceduralTexture::~ProceduralTexture(){

}
//////////////////////////////////////////////////////////////////////////////////////////////////
CheckerTexture::CheckerTexture() : ProceduralTexture(){

}

CheckerTexture::~CheckerTexture(){

}

void CheckerTexture::setColor1(const float r, const float g, const float b){

	m_color1 = Color(r, g, b);
}
void CheckerTexture::setColor1(const float c){

	m_color1 = Color(c, c, c);
}
void CheckerTexture::setColor1(const Color& c){

	m_color1 = c;
}

void CheckerTexture::setColor2(const float r, const float g, const float b){

	m_color2 = Color(r, g, b);
}

void CheckerTexture::setColor2(const float c){

	m_color2 = Color(c, c, c);
}

void CheckerTexture::setColor2(const Color& c){

	m_color2 = c;
}

void CheckerTexture::setLineColor(const float r, const float g, const float b){

	m_lineColor = Color(r, g, b);
}

void CheckerTexture::setLineColor(const float c){

	m_lineColor = Color(c, c, c);
}

void CheckerTexture::setLineColor(const Color& c){

	m_lineColor = c;
}
///////////////////////////////////////////////////////////////////////////////////////////////
Checker3D::Checker3D() : CheckerTexture(){}

Checker3D::~Checker3D(){}

Color Checker3D::getColor(const Vector3f& pos){

	float tmpEps = -0.000187453738;	// small random number
	float x = pos[0] + tmpEps;
	float y = pos[1] + tmpEps;
	float z = pos[2] + tmpEps;

	int ix = floor(x / m_size);
	int iy = floor(y / m_size);
	int iz = floor(z / m_size);

	float fx = x / m_size - ix;
	float fy = y / m_size - iy;
	float fz = z / m_size - iz;

	float width = 0.5 * m_outlineWidth / m_size;

	bool in_outline = (fx < width || fx > 1.0 - width) || (fy < width || fy > 1.0 - width) || (fz < width || fz > 1.0 - width);

	if ((ix + iy + iz) % 2 == 0) {
		if (!in_outline)
			return (m_color1);
	}else {

		if (!in_outline)
			return (m_color2);
	}

	return m_lineColor;
}

void Checker3D::setSize(const float size){

	m_size = size;
}

void Checker3D::setOutlineWidth(const float width){

	m_outlineWidth = width;
}

///////////////////////////////////////////////////////////////////////////////////////////////
PlaneChecker::PlaneChecker() : CheckerTexture(){}

PlaneChecker::~PlaneChecker(){}

Color PlaneChecker::getColor(const Vector3f& pos){
	
	
	float x = pos[0];
	float z = pos[2];
	int ix = floor(x / m_size);
	int iz = floor(z / m_size);
	float fx = x / m_size - ix;
	float fz = z / m_size - iz;
	float width = 0.5 * m_outlineWidth / m_size;
	bool in_outline = (fx < width || fx > 1.0 - width) || (fz < width || fz > 1.0 - width);

	if ((ix + iz) % 2 == 0) {
		if (!in_outline)
			return (m_color1);
	}else {

		if (!in_outline)
			return (m_color2);
	}

	return m_lineColor;

}

void PlaneChecker::setSize(const float size){

	m_size = size;
}

void PlaneChecker::setOutlineWidth(const float width){

	m_outlineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
SphereChecker::SphereChecker() : CheckerTexture(){}

SphereChecker::~SphereChecker(){}

Color SphereChecker::getColor(const Vector3f& pos){

	float theta = acos(pos[1]);

	float phi = atan2(pos[2], pos[0]);
	if (phi < 0.0)
		phi += 2.0 * PI;

	float phi_size = 2 * PI / m_numHorizontalCheckers;   	// in radians - azimuth angle
	float theta_size = PI / m_numVerticalCheckers;   		// in radians - polar angle

	int i_phi = floor(phi / phi_size);
	int i_theta = floor(theta / theta_size);

	float f_phi = phi / phi_size - i_phi;
	float f_theta = theta / theta_size - i_theta;

	float phi_line_width = 0.5 * m_verticalLineWidth;
	float theta_line_width = 0.5 * m_horizontalLineWidth;

	bool in_outline = (f_phi < phi_line_width || f_phi > 1.0 - phi_line_width) ||
		(f_theta < theta_line_width || f_theta > 1.0 - theta_line_width);

	if ((i_phi + i_theta) % 2 == 0) {
		if (!in_outline)

			//std::cout << m_color2.r << "  " << m_color2.g << "  " << m_color2.b << std::endl;

			return (m_color2);

	}else {
		if (!in_outline)

			

			return (m_color1);
	}

	return m_lineColor;
}

void SphereChecker::setNumHorizontalCheckers(const int numHorizontal){

	m_numHorizontalCheckers = numHorizontal;
}

void SphereChecker::setNumVerticalCheckers(const int numVertical){

	m_numVerticalCheckers = numVertical;
}

void SphereChecker::setHorizontalLineWidth(const float width){

	m_horizontalLineWidth = width;
}

void SphereChecker::setVerticalLineWidth(const float width){

	m_verticalLineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CylinderChecker::CylinderChecker() : CheckerTexture(){}

CylinderChecker::~CylinderChecker(){}

Color CylinderChecker::getColor(const Vector3f& pos){

	float v = (pos[1] + 1.0) / 2.0;

	float phi = atan2(pos[2], pos[0]);
	if (phi < 0.0)
		phi += 2.0 * PI;

	float phi_size = 2 * PI / m_numHorizontalCheckers;   	// in radians - azimuth angle
	float theta_size = 1.0 / m_numVerticalCheckers;   		// in radians - polar angle

	int i_phi = floor(phi / phi_size);
	int i_v = floor(v / theta_size);

	float f_phi = phi / phi_size - i_phi;
	float f_v = v/ theta_size - i_v;

	float phi_line_width = 0.5 * m_verticalLineWidth;
	float theta_line_width = 0.5 * m_horizontalLineWidth;

	bool in_outline = (f_phi < phi_line_width || f_phi > 1.0 - phi_line_width) ||
		(f_v < theta_line_width || f_v > 1.0 - theta_line_width);

	if ((i_phi + i_v) % 2 == 0) {
		if (!in_outline)
			return (m_color2);

	}else {
		if (!in_outline)
			return (m_color1);
	}

	return m_lineColor;
}

void CylinderChecker::setNumHorizontalCheckers(const int numHorizontal){

	m_numHorizontalCheckers = numHorizontal;
}

void CylinderChecker::setNumVerticalCheckers(const int numVertical){

	m_numVerticalCheckers = numVertical;
}

void CylinderChecker::setHorizontalLineWidth(const float width){

	m_horizontalLineWidth = width;
}

void CylinderChecker::setVerticalLineWidth(const float width){

	m_verticalLineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
DiskChecker::DiskChecker() : CheckerTexture(){}

DiskChecker::~DiskChecker(){}

Color DiskChecker::getColor(const Vector3f& pos){

	float x = pos[0];
	float z = pos[2];

	float phi = atan2(pos[2], pos[0]);
	if (phi < 0.0)
		phi += TWO_PI;

	
	float phi_size = 2 * PI / m_numAngularCheckers;

	float radial = sqrt(x * x + z * z);
	float radial_size = 1.0 / m_numRadialCheckers;

	int i_phi = floor(phi / phi_size);
	int i_radial = floor(radial / radial_size);
	
	float f_phi = phi / phi_size - i_phi;
	float f_radial = radial / radial_size - i_radial;


	float phi_line_width = 0.5 * m_angularLineWidth;
	float radial_line_width = 0.5 * m_radialLineWidth;

	bool in_outline = (f_radial < radial_line_width || f_radial > 1.0 - radial_line_width)||
				      (f_phi < phi_line_width || f_phi > 1.0 - phi_line_width);

	if ((i_phi + i_radial) % 2 == 0) {
		if (!in_outline)
			return (m_color1);

	}else {
		if (!in_outline)
			return (m_color2);
	}

	return m_lineColor;
}

void DiskChecker::setNumAngularCheckers(const int numAngular){

	m_numAngularCheckers = numAngular;
}

void DiskChecker::setNumRadialCheckers(const int numRadial){

	m_numRadialCheckers = numRadial;
}

void DiskChecker::setAngularLineWidth(const float width){

	m_angularLineWidth = width;
}

void DiskChecker::setRadialLineWidth(const float width){

	m_radialLineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
ConeChecker::ConeChecker() : CheckerTexture(){}

ConeChecker::~ConeChecker(){}

Color ConeChecker::getColor(const Vector3f& pos){

	float len = sqrt(pos[0] * pos[0] + pos[2] * pos[2]);

	float v = 1.0 - len;

	float phi = atan2(pos[2], pos[0]);
	if (phi < 0.0)
		phi += 2.0 * PI;



	float phi_size = 2 * PI / m_numHorizontalCheckers;   	// in radians - azimuth angle
	float theta_size = 1.0 / m_numVerticalCheckers;   		// in radians - polar angle

	int i_phi = floor(phi / phi_size);
	int i_v = floor(v / theta_size);

	float f_phi = phi / phi_size - i_phi;
	float f_v = v / theta_size - i_v;

	float phi_line_width = 0.5 * m_verticalLineWidth;
	float theta_line_width = 0.5 * m_horizontalLineWidth;

	bool in_outline = (f_phi < phi_line_width || f_phi > 1.0 - phi_line_width) ||
		(f_v < theta_line_width || f_v > 1.0 - theta_line_width);

	if ((i_phi + i_v) % 2 == 0) {
		if (!in_outline)
			return (m_color1);

	}
	else {
		if (!in_outline)
			return (m_color2);
	}

	return m_lineColor;
}

void ConeChecker::setNumHorizontalCheckers(const int numHorizontal){

	m_numHorizontalCheckers = numHorizontal;
}

void ConeChecker::setNumVerticalCheckers(const int numVertical){

	m_numVerticalCheckers = numVertical;
}

void ConeChecker::setHorizontalLineWidth(const float width){

	m_horizontalLineWidth = width;
}

void ConeChecker::setVerticalLineWidth(const float width){

	m_verticalLineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
RectangleChecker::RectangleChecker() : CheckerTexture(){

	m_generic = true;
}

RectangleChecker::~RectangleChecker(){}

void RectangleChecker::setAttributes(Vector3f pos, Vector3f a, Vector3f b){

	m_pos = pos;
	m_a = a;
	m_b = b;
	m_sqA = a.sqMagnitude();
	m_sqB = b.sqMagnitude();

	m_generic = false;
}

Color RectangleChecker::getColor(const Vector3f& pos){

	float u, v;

	if (m_generic){
		u = (pos[0] + 1.0) * 0.5;
		v = (pos[2] + 1.0) * 0.5;
		
	}else{

		Vector3f transformedPos = pos - m_pos;
		v = Vector3f::dot(m_b, transformedPos) * (1.0 / m_sqB);
		u = 1.0 - Vector3f::dot(m_a, transformedPos) * (1.0 / m_sqA);
	}


	float v_size = 1.0 / m_numXCheckers;   	// in radians - azimuth angle
	float u_size = 1.0 / m_numZCheckers;   		// in radians - polar angle

	int i_v = floor(v * m_numXCheckers);
	int i_u = floor(u * m_numZCheckers);

	float f_v = v / v_size - i_v;
	float f_u = u / u_size - i_u;
	

	float v_line_width = 0.5 * m_ZLineWidth;
	float u_line_width = 0.5 * m_XLineWidth;
	

	bool in_outline = (f_u < u_line_width || f_u > 1.0 - u_line_width) || (f_v < v_line_width || f_v > 1.0 - v_line_width);

	
	if ((i_v + i_u) % 2 == 0) {
		if (!in_outline)
			return (m_color2);
	}else {
		if (!in_outline)
			return (m_color1);
	}

	return m_lineColor;
}

void RectangleChecker::setNumXCheckers(const int numX){

	m_numXCheckers = numX;
}

void RectangleChecker::setNumZCheckers(const int numZ){

	m_numZCheckers = numZ;
}

void RectangleChecker::setXLineWidth(const float width){

	m_XLineWidth = width;
}

void RectangleChecker::setZLineWidth(const float width){

	m_ZLineWidth = width;
}
///////////////////////////////////////////////////////////////////////////////////////////////
TorusChecker::TorusChecker() : CheckerTexture(){

	m_radius = 1.0;
}

TorusChecker::~TorusChecker(){

}

Color TorusChecker::getColor(const Vector3f& pos){

	

	// Determine its angle from the x-axis.
	float len = sqrt(pos[2] * pos[2] + pos[0] * pos[0]);
	float x = len - m_radius;
	float theta = atan2(pos[1], x);
	

	float phi = atan2(pos[2], pos[0]);
	

	float phi_size = 2 * PI / m_numHorizontalCheckers;   	// in radians - azimuth angle
	float theta_size = 2 * PI / m_numVerticalCheckers;   		// in radians - polar angle

	int i_phi = floor(phi / phi_size);
	int i_theta = floor(theta / theta_size);

	float f_phi = phi / phi_size - i_phi;
	float f_theta = theta / theta_size - i_theta;

	float phi_line_width = 0.5 * m_verticalLineWidth;
	float theta_line_width = 0.5 * m_horizontalLineWidth;

	bool in_outline = (f_phi < phi_line_width || f_phi > 1.0 - phi_line_width) ||
		(f_theta < theta_line_width || f_theta > 1.0 - theta_line_width);

	if ((i_phi + i_theta) % 2 == 0) {
		if (!in_outline)
			return (m_color2);

	}
	else {
		if (!in_outline)
			return (m_color1);
	}

	return m_lineColor;
}

void TorusChecker::setRadius(float radius){

	m_radius = radius;
}

void TorusChecker::setNumHorizontalCheckers(const int numHorizontal){

	m_numHorizontalCheckers = numHorizontal;
}

void TorusChecker::setNumVerticalCheckers(const int numVertical){

	m_numVerticalCheckers = numVertical;
}

void TorusChecker::setHorizontalLineWidth(const float width){

	m_horizontalLineWidth = width;
}

void TorusChecker::setVerticalLineWidth(const float width){

	m_verticalLineWidth = width;
}

