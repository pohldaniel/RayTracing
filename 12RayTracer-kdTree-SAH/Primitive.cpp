#include <array>
#include "KDTree.h"


bool BBox::intersect(const Ray& ray) {
	double ox = ray.origin[0]; double oy = ray.origin[1]; double oz = ray.origin[2];
	double dx = ray.direction[0]; double dy = ray.direction[1]; double dz = ray.direction[2];

	double tx_min, ty_min, tz_min;
	double tx_max, ty_max, tz_max;

	double a = 1.0 / dx;
	if (a >= 0) {
		tx_min = (m_Pos[0] - ox) * a;
		tx_max = (m_Size[0] + m_Pos[0] - ox) * a;
	}
	else {
		tx_min = (m_Size[0] + m_Pos[0] - ox) * a;
		tx_max = (m_Pos[0] - ox) * a;
	}

	double b = 1.0 / dy;
	if (b >= 0) {
		ty_min = (m_Pos[1] - oy) * b;
		ty_max = (m_Size[1] + m_Pos[1] - oy) * b;
	}
	else {
		ty_min = (m_Size[1] + m_Pos[1] - oy) * b;
		ty_max = (m_Pos[1] - oy) * b;
	}

	double c = 1.0 / dz;
	if (c >= 0) {
		tz_min = (m_Pos[2] - oz) * c;
		tz_max = (m_Size[2] + m_Pos[2] - oz) * c;
	}
	else {
		tz_min = (m_Size[2] + m_Pos[2] - oz) * c;
		tz_max = (m_Pos[2] - oz) * c;
	}

	double t0, t1;

	// find largest entering t value

	if (tx_min > ty_min)
		t0 = tx_min;
	else
		t0 = ty_min;

	if (tz_min > t0)
		t0 = tz_min;

	// find smallest exiting t value

	if (tx_max < ty_max)
		t1 = tx_max;
	else
		t1 = ty_max;

	if (tz_max < t1)
		t1 = tz_max;

	if (t0 < t1 && t1 > 0.0001){
		return true;
	}
	return false;

}



Primitive::Primitive() {

	Primitive::color = Color(0.0, 1.0, 0.0);
	Primitive::normal = Vector3f(0.0, 1.0, 0.0);
	Primitive::orientable = false;
	Primitive::T.identity();

}

Primitive::Primitive(const Color &color, const Vector3f &normal){

	Primitive::color = color;
	Primitive::normal = normal;

	orientable = false;
	Primitive::T.identity();

}

Primitive::~Primitive(){

}
//////////////////////////////////////////////////////////////////////////////////////////////////
OrientablePrimitive::OrientablePrimitive() :Primitive(){
	orientable = true;

}


OrientablePrimitive::OrientablePrimitive(const Color& color, const Vector3f &normal) : Primitive(color, normal){
	orientable = true;

}

OrientablePrimitive::~OrientablePrimitive(){


}

void OrientablePrimitive::rotate(const Vector3f &axis, float degrees){

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degrees);

	T = rotMtx *T;


}

void OrientablePrimitive::translate(float dx, float dy, float dz){

	Matrix4f transMtx;
	transMtx.translate(dx, dy, dz);
	T = transMtx *T;
	//T *= transMtx;
}

void OrientablePrimitive::scale(float a, float b, float c){

	Matrix4f scaleMtx;
	scaleMtx.scale(a , b , c);
	T = scaleMtx *T;

}
//////////////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle() {

}

Triangle::Triangle(Vector3f &a_V1, 
				   Vector3f &a_V2, 
				   Vector3f &a_V3, 
				   Vector3f &normal) : Primitive(Color(0.0, 1.0 ,0.0), normal)
{
	
	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	

	// init precomp
	Vector3f A = m_a;
	Vector3f B = m_b;
	Vector3f C = m_c;
	Vector3f c = B - A;
	Vector3f b = C - A;
	Triangle::normal = Vector3f::cross(b, c);
	int u, v;
	if (fabs(Triangle::normal[0]) > fabs(Triangle::normal[1]))
	{
		if (fabs(Triangle::normal[0]) > fabs(Triangle::normal[2])) k = 0; else k = 2;
	}
	else
	{
		if (fabs(Triangle::normal[1]) > fabs(Triangle::normal[2])) k = 1; else k = 2;
	}
	u = (k + 1) % 3;
	v = (k + 2) % 3;
	// precomp
	double krec = 1.0f / Triangle::normal[k];
	nu = Triangle::normal[u] * krec;
	nv = Triangle::normal[v] * krec;
	nd = Vector3f::dot(Triangle::normal, A)* krec;
	// first line equation
	double reci = 1.0f / (b[u] * c[v] - b[v] * c[u]);
	bnu = b[u] * reci;
	bnv = -b[v] * reci;
	// second line equation
	cnu = c[v] * reci;
	cnv = -c[u] * reci;
	// finalize normal
	Triangle::normal.normalize();

}

Triangle::~Triangle()
{

}

#define FINDMINMAX( x0, x1, x2, min, max ) \
	min = max = x0; if (x1<min) min = x1; if (x1>max) max = x1; if (x2<min) min = x2; if (x2>max) max = x2;
// X-tests
#define AXISTEST_X01( a, b, fa, fb )											\
	p0 = a * v0[1] - b * v0[2], p2 = a * v2[1] - b * v2[2]; \
if (p0 < p2) { min = p0; max = p2; }else { min = p2; max = p0; }			\
	rad = fa * a_BoxHalfsize[1] + fb * a_BoxHalfsize[2];				\
if (min > rad || max < -rad) return 0;
#define AXISTEST_X2( a, b, fa, fb )												\
	p0 = a * v0[1] - b * v0[2], p1 = a * v1[1] - b * v1[2];	\
if (p0 < p1) { min = p0; max = p1; }else { min = p1; max = p0; }			\
	rad = fa * a_BoxHalfsize[1] + fb * a_BoxHalfsize[2];				\
if (min>rad || max<-rad) return 0;
	// Y-tests
#define AXISTEST_Y02( a, b, fa, fb )											\
	p0 = -a * v0[0] + b * v0[2], p2 = -a * v2[0] + b * v2[2]; \
if (p0 < p2) { min = p0; max = p2; }else { min = p2; max = p0; }			\
	rad = fa * a_BoxHalfsize[0] + fb * a_BoxHalfsize[2];				\
if (min > rad || max < -rad) return 0;
#define AXISTEST_Y1( a, b, fa, fb )												\
	p0 = -a * v0[0] + b * v0[2], p1 = -a * v1[0] + b * v1[2]; \
if (p0 < p1) { min = p0; max = p1; }else { min = p1; max = p0; }			\
	rad = fa * a_BoxHalfsize[0] + fb * a_BoxHalfsize[2];				\
if (min > rad || max < -rad) return 0;
// Z-tests
#define AXISTEST_Z12( a, b, fa, fb )											\
	p1 = a * v1[0] - b * v1[1], p2 = a * v2[0] - b * v2[1]; \
if (p2 < p1) { min = p2; max = p1; }else { min = p1; max = p2; }			\
	rad = fa * a_BoxHalfsize[0] + fb * a_BoxHalfsize[1];				\
if (min > rad || max < -rad) return 0;
#define AXISTEST_Z0( a, b, fa, fb )												\
	p0 = a * v0[0] - b * v0[1], p1 = a * v1[0] - b * v1[1];	\
if (p0 < p1) { min = p0; max = p1; }else { min = p1; max = p0; }			\
	rad = fa * a_BoxHalfsize[0] + fb * a_BoxHalfsize[1];				\
if (min > rad || max < -rad) return 0;

unsigned int modulo[] = { 0, 1, 2, 0, 1 };
void Triangle::hit(const Ray& a_Ray, Hit &hit)
{
	
	
#define ku modulo[k + 1]
#define kv modulo[k + 2]
		Vector3f O = a_Ray.origin, D = a_Ray.direction, A = m_a;
		const double lnd = 1.0f / (D[k] + nu * D[ku] + nv * D[kv]);
		const double t = (nd - O[k] - nu * O[ku] - nv * O[kv]) * lnd;
		if (!(hit.t > t && t > 0)) return;
		double hu = O[ku] + t * D[ku] - A[ku];
		double hv = O[kv] + t * D[kv] - A[kv];
		double beta = m_U = hv * bnu + hu * bnv;
		if (beta < 0) return;
		double gamma = m_V = hu * cnu + hv * cnv;
		if (gamma < 0) return;
		if ((m_U + m_V) > 1) return;
		hit.t = t;
		// Backface Culling

		(Vector3f::dot(D, Triangle::normal) > 0) ? hit.hitObject = true : hit.hitObject = true;
		return;
	
}


bool Triangle::PlaneBoxOverlap(Vector3f& a_Normal, Vector3f& a_Vert, Vector3f& a_MaxBox)
{
	Vector3f vmin, vmax;
	for (int q = 0; q < 3; q++)
	{
		float v = a_Vert[q];
		if (a_Normal[q] > 0.0f)
		{
			vmin[q] = -a_MaxBox[q] - v;
			vmax[q] = a_MaxBox[q] - v;
		}
		else
		{
			vmin[q] = a_MaxBox[q] - v;
			vmax[q] = -a_MaxBox[q] - v;
		}
	}
	if (Vector3f::dot(a_Normal, vmin) > 0.0f) return false;
	if (Vector3f::dot(a_Normal, vmax) >= 0.0f) return true;
	return false;
}

bool Triangle::IntersectTriBox(Vector3f& a_BoxCentre, Vector3f& a_BoxHalfsize, Vector3f& a_V0, Vector3f& a_V1, Vector3f& a_V2)
{
	Vector3f v0, v1, v2, normal, e0, e1, e2;
	float min, max, p0, p1, p2, rad, fex, fey, fez;
	v0 = a_V0 - a_BoxCentre;
	v1 = a_V1 - a_BoxCentre;
	v2 = a_V2 - a_BoxCentre;
	e0 = v1 - v0, e1 = v2 - v1, e2 = v0 - v2;
	fex = fabsf(e0[0]);
	fey = fabsf(e0[1]);
	fez = fabsf(e0[2]);
	AXISTEST_X01(e0[2], e0[1], fez, fey);
	AXISTEST_Y02(e0[2], e0[0], fez, fex);
	AXISTEST_Z12(e0[1], e0[0], fey, fex);
	fex = fabsf(e1[0]);
	fey = fabsf(e1[1]);
	fez = fabsf(e1[2]);
	AXISTEST_X01(e1[2], e1[1], fez, fey);
	AXISTEST_Y02(e1[2], e1[0], fez, fex);
	AXISTEST_Z0(e1[1], e1[0], fey, fex);
	fex = fabsf(e2[0]);
	fey = fabsf(e2[1]);
	fez = fabsf(e2[2]);
	AXISTEST_X2(e2[2], e2[1], fez, fey);
	AXISTEST_Y1(e2[2], e2[0], fez, fex);
	AXISTEST_Z12(e2[1], e2[0], fey, fex);
	FINDMINMAX(v0[0], v1[0], v2[0], min, max);
	if (min > a_BoxHalfsize[0] || max < -a_BoxHalfsize[0]) return false;
	FINDMINMAX(v0[1], v1[1], v2[1], min, max);
	if (min > a_BoxHalfsize[1] || max < -a_BoxHalfsize[1]) return false;
	FINDMINMAX(v0[2], v1[2], v2[2], min, max);
	if (min > a_BoxHalfsize[2] || max < -a_BoxHalfsize[2]) return false;
	normal = Vector3f::cross(e0, e1);
	if (!PlaneBoxOverlap(normal, v0, a_BoxHalfsize)) return false;
	return true;
}


bool Triangle::IntersectBox(BBox& a_Box)
{

	return IntersectTriBox(a_Box.GetPos() + a_Box.GetSize() * 0.5f, a_Box.GetSize() * 0.5f,
		m_a, m_b, m_c);

}

void Triangle::CalculateRange(double& a_Pos1, double& a_Pos2, int a_Axis)
{

	Vector3f pos1 = m_a;
	a_Pos1 = pos1[a_Axis], a_Pos2 = pos1[a_Axis];


	Vector3f pos = m_a;
	if (pos[a_Axis] < a_Pos1) a_Pos1 = pos[a_Axis];
	if (pos[a_Axis] > a_Pos2) a_Pos2 = pos[a_Axis];

	pos = m_b;
	if (pos[a_Axis] < a_Pos1) a_Pos1 = pos[a_Axis];
	if (pos[a_Axis] > a_Pos2) a_Pos2 = pos[a_Axis];

	pos = m_c;
	if (pos[a_Axis] < a_Pos1) a_Pos1 = pos[a_Axis];
	if (pos[a_Axis] > a_Pos2) a_Pos2 = pos[a_Axis];



}

Vector3f Triangle::GetPos(int a_Idx){
	if (a_Idx == 0) return m_a;
	else if (a_Idx == 1) return m_b;
	else if (a_Idx == 2) return m_c;

}



//////////////////////////////////////////////////////////////////////////////////////////////////
Sphere::Sphere(){

}

Sphere::Sphere(Vector3f& a_Centre, double a_Radius, Color color) :Primitive(color , Vector3f(0.0, 1.0, 0.0) )
{
	m_Centre = a_Centre;
	m_SqRadius = a_Radius * a_Radius;
	m_Radius = a_Radius;
	m_RRadius = 1.0f / a_Radius;

	// set vectors for texture mapping
	m_Vn = Vector3f(0, 1, 0);
	m_Ve = Vector3f(1, 0, 0);
	m_Vc = Vector3f::cross(m_Vn, m_Ve);
}

Sphere::~Sphere(){

}

Vector3f Sphere::GetNormal(Vector3f& a_Pos)
{
	
		return (a_Pos - m_Centre) * m_RRadius;

	
}

void Sphere::hit(const Ray &ray, Hit &hit) {

	//Use position - origin to get a negative b
	Vector3f L = m_Centre - ray.origin;


	float b = Vector3f::dot(L, ray.direction);
	float c = Vector3f::dot(L, L) - m_SqRadius;

	float d = b*b - c;
	if (d < 0.00001) {
		hit.hitObject = false;
		return;
	}
	//----------------------------------
	float result = -1.0;

	result = b - sqrt(d);
	if (result < 0.0){

		result = b + sqrt(d);

	}


	if (result > 0.0){

		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}

bool Sphere::IntersectSphereBox(Vector3f& a_Centre, BBox& a_Box){

	float dmin = 0;
	Vector3f spos = a_Centre;
	Vector3f bpos = a_Box.GetPos();
	Vector3f bsize = a_Box.GetSize();
	for (int i = 0; i < 3; i++)
	{
		if (spos[i] < bpos[i])
		{
			dmin = dmin + (spos[i] - bpos[i]) * (spos[i] - bpos[i]);
		}
		else if (spos[i] > (bpos[i] + bsize[i]))
		{
			dmin = dmin + (spos[i] - (bpos[i] + bsize[i])) * (spos[i] - (bpos[i] + bsize[i]));
		}
	}
	return (dmin <= m_SqRadius);
}

bool Sphere::IntersectBox(BBox& a_Box){

	return IntersectSphereBox(m_Centre, a_Box);
}
void  Sphere::CalculateRange(double& a_Pos1, double& a_Pos2, int a_Axis){

	a_Pos1 = m_Centre[a_Axis] - m_Radius;
	a_Pos2 = m_Centre[a_Axis] + m_Radius;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane() :Primitive(){

	Plane::normal = Vector3f(0.0, 1.0, 0.0);
	Plane::distance = -1.0;

}

Plane::Plane(Vector3f normal, float distance, Color color) :Primitive(color, normal){

	Plane::distance = distance;

}

void Plane::hit(const Ray &ray, Hit &hit){

	float result = -1;

	result = (distance - Vector3f::dot(normal, ray.origin)) / Vector3f::dot(normal, ray.direction);

	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////



Torus::Torus() :OrientablePrimitive(){

	Torus::a = 1.0;
	Torus::b = 0.5;
}

Torus::Torus(float a, float b, Color color) :OrientablePrimitive(color, Vector3f(0.0 , 1.0 ,0.0)){

	Torus::a = a;
	Torus::b = b;
}

Torus::~Torus(){

}

void Torus::hit(const Ray &ray, Hit &hit){

	double Ra2 = Torus::a*Torus::a;
	double ra2 = Torus::b*Torus::b;


	double m = Vector3f::dot(ray.origin, ray.origin);
	double n = Vector3f::dot(ray.origin, ray.direction);

	double k = (m - ra2 - Ra2) / 2.0;
	double a = n;
	double b = n*n + Ra2*ray.direction[0] * ray.direction[0] + k;
	double c = k*n + Ra2*ray.origin[0] * ray.direction[0];
	double d = k*k + Ra2*ray.origin[0] * ray.origin[0] - Ra2*ra2;

	//----------------------------------

	double p = -3.0*a*a + 2.0*b;
	double q = 2.0*a*a*a - 2.0*a*b + 2.0*c;
	double r = -3.0*a*a*a*a + 4.0*a*a*b - 8.0*a*c + 4.0*d;
	p /= 3.0;
	r /= 3.0;
	double Q = p*p + r;
	double R = 3.0*r*p - p*p*p - q*q;

	double h = R*R - Q*Q*Q;
	double z = 0.0;

	// discriminant > 0
	if (h < 0.0)
	{
		double sQ = sqrt(Q);
		z = 2.0*sQ*cos(acos(R / (sQ*Q)) / 3.0);
	}
	// discriminant < 0
	else
	{
		double sQ = pow(sqrt(h) + abs(R), 1.0 / 3.0);
		if (R < 0.0){
			z = -(sQ + Q / sQ);
		}
		else{
			z = (sQ + Q / sQ);
		}
	}

	z = p - z;

	//----------------------------------

	double d1 = z - 3.0*p;
	double d2 = z*z - 3.0*r;
	double d1o2 = d1 / 2.0;

	if (abs(d1)< 0.0001)
	{
		if (d2 < 0.0)  {
			hit.hitObject = false;
			return;
		}
		d2 = sqrt(d2);
	}
	else
	{
		if (d1 < 0.0) {
			hit.hitObject = false;
			return;
		}
		d1 = sqrt(d1 / 2.0);
		d2 = q / d1;
	}

	//----------------------------------

	double result = -1.0;
	double result2 = -1.0;

	double t1;
	double t2;

	h = d1o2 - z + d2;
	if (h>0.0)
	{
		h = sqrt(h);

		t1 = -d1 - h - a;
		t2 = -d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result = min(t1, t2);
		}
		else if (t1 > 0) {
			result = t1;
		}
		else if (t2 > 0){
			result = t2;
		}

	}


	h = d1o2 - z - d2;
	if (h>0.0)
	{
		h = sqrt(h);
		t1 = d1 - h - a;
		t2 = d1 + h - a;

		if (t1 > 0.0 && t2 > 0.0) {
			result2 = min(t1, t2);
		}
		else if (t1 > 0) {
			result2 = t1;
		}
		else if (t2 > 0){
			result2 = t2;
		}
	}

	if (result2 > 0.0 && result > 0.0){
		result = min(result, result2);

	}
	else if (result2 > 0.0) result = result2;



	if (result > 0.0){
		hit.t = result;
		hit.hitObject = true;
		return;
	}

	hit.hitObject = false;
	return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh() 
{


	Mesh::color = Color(0.0, 1.0, 0.0);

	m_Mod = new int[64];
	m_Mod = (int*)((((unsigned long)m_Mod) + 32) & (0xffffffff - 31));
	m_Mod[0] = 0, m_Mod[1] = 1, m_Mod[2] = 2, m_Mod[3] = 0, m_Mod[4] = 1;
	m_Stack = new kdstack[64];
	m_Stack = (kdstack*)((((unsigned long)m_Stack) + 32) & (0xffffffff - 31));
}




Mesh::~Mesh()
{

}


int Mesh::traverse(const Ray& a_Ray, Hit &hit)
{
	double tnear = 0, tfar = hit.t, t;
	int retval = 0;
	Vector3f p1 = m_Extends.GetPos();
	Vector3f p2 = m_Extends.GetSize() + p1;
	Vector3f D = a_Ray.direction, O = a_Ray.origin;
	for (int i = 0; i < 3; i++) if (D[i] < 0)
	{
		if (O[i] < p1[i]) return 0;
	}
	else if (O[i] > p2[i]) return 0;
	// clip ray segment to box
	for (int i = 0; i < 3; i++)
	{
		double pos = O[i] + tfar * D[i];
		if (D[i] < 0)
		{
			// clip end point
			if (pos < p1[i]) tfar = tnear + (tfar - tnear) * ((O[i] - p1[i]) / (O[i] - pos));
			// clip start point
			if (O[i] > p2[i]) tnear += (tfar - tnear) * ((O[i] - p2[i]) / (tfar * D[i]));
		}
		else
		{
			// clip end point
			if (pos > p2[i]) tfar = tnear + (tfar - tnear) * ((p2[i] - O[i]) / (pos - O[i]));
			// clip start point
			if (O[i] < p1[i]) tnear += (tfar - tnear) * ((p1[i] - O[i]) / (tfar * D[i]));
		}
		if (tnear > tfar) return 0;
	}
	// init stack
	int entrypoint = 0, exitpoint = 1;
	// init traversal
	KdTreeNode* farchild, *currnode;
	currnode = m_KdTree->GetRoot();
	m_Stack[entrypoint].t = tnear;
	if (tnear > 0.0f) m_Stack[entrypoint].pb = O + D * tnear;
	else m_Stack[entrypoint].pb = O;
	m_Stack[exitpoint].t = tfar;
	m_Stack[exitpoint].pb = O + D * tfar;
	m_Stack[exitpoint].node = 0;
	// traverse kd-tree
	while (currnode)
	{
		while (!currnode->IsLeaf())
		{
			double splitpos = currnode->GetSplitPos();
			int axis = currnode->GetAxis();
			if (m_Stack[entrypoint].pb[axis] <= splitpos)
			{
				if (m_Stack[exitpoint].pb[axis] <= splitpos)
				{
					currnode = currnode->GetLeft();
					continue;
				}
				if (m_Stack[exitpoint].pb[axis] == splitpos)
				{
					currnode = currnode->GetRight();
					continue;
				}
				currnode = currnode->GetLeft();
				farchild = currnode + 1; // GetRight();
			}
			else
			{
				if (m_Stack[exitpoint].pb[axis] > splitpos)
				{
					currnode = currnode->GetRight();
					continue;
				}
				farchild = currnode->GetLeft();
				currnode = farchild + 1; // GetRight();
			}
			t = (splitpos - O[axis]) / D[axis];
			int tmp = exitpoint++;
			if (exitpoint == entrypoint) exitpoint++;
			m_Stack[exitpoint].prev = tmp;
			m_Stack[exitpoint].t = t;
			m_Stack[exitpoint].node = farchild;
			m_Stack[exitpoint].pb[axis] = splitpos;
			int nextaxis = m_Mod[axis + 1];
			int prevaxis = m_Mod[axis + 2];
			m_Stack[exitpoint].pb[nextaxis] = O[nextaxis] + t * D[nextaxis];
			m_Stack[exitpoint].pb[prevaxis] = O[prevaxis] + t * D[prevaxis];
		}
		ObjectList* list = currnode->GetList();

		Hit hit2;
		hit2.t = m_Stack[exitpoint].t;
		while (list)
		{
			Triangle* pr = list->GetPrimitive();
			int result;
			pr->hit(a_Ray, hit2);
			if (hit2.hitObject)
			{
				retval = result;
				hit.t = hit2.t;
				//a_Prim = pr;
			}
			list = list->GetNext();

			
		}
		if (retval) return retval;
		entrypoint = exitpoint;
		currnode = m_Stack[exitpoint].node;
		exitpoint = m_Stack[entrypoint].prev;
	}
	return 0;
}

void Mesh::hit(const Ray& a_Ray, Hit &hit)
{
	if (!m_Extends.intersect(a_Ray)){

		//hit.color = Color(1.0, 0.0, 0.0);
		return;
		
	}else{

		if (!hit.hitObject){

			hit.t = 1.0E10;

		}
		// find the nearest intersection
		if (!(traverse(a_Ray, hit))){
			return;
			}
			hit.hitObject = true;
			
			return;
	}
	
	
}

bool Mesh::loadObject(const char* filename){
	
	return loadObject(filename, 1.0);
}

bool Mesh::loadObject(const char* filename, float scale){


	std::vector<std::string*>coord;
	std::vector<Vector3f*> vertex;
	std::vector<std::array<int, 5>> vec;
	std::vector<Vector3f*> normals;
	std::ifstream in(filename);

	if (!in.is_open()){

		std::cout << "File not found" << std::endl;
		return false;
	}

	std::string line;
	while (getline(in, line)){
		coord.push_back(new std::string(line));

	}
	in.close();

	for (int i = 0; i < coord.size(); i++){
		if ((*coord[i])[0] == '#')
			continue;
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == ' '){


			float tmpx, tmpy, tmpz;
			sscanf(coord[i]->c_str(), "v %f %f %f", &tmpx, &tmpy, &tmpz);
			vertex.push_back(new Vector3f(tmpx*scale, tmpy * scale, tmpz * scale));
		}
		else if ((*coord[i])[0] == 'v' && (*coord[i])[1] == 'n'){
			float tmpx, tmpy, tmpz;
			sscanf(coord[i]->c_str(), "vn %f %f %f", &tmpx, &tmpy, &tmpz);
			normals.push_back(new Vector3f(tmpx, tmpy, tmpz));
		}
		else if ((*coord[i])[0] == 'f'){

			int a, b, c, d, e;

			if (std::count(coord[i]->begin(), coord[i]->end(), ' ') == 3){

				sscanf(coord[i]->c_str(), "f %d//%d %d//%d %d//%d", &a, &b, &c, &b, &d, &b);

				vec.push_back({ { b, a, c, d, 0 } });
			}
			else{

				sscanf(coord[i]->c_str(), "f %d//%d %d//%d %d//%d ", &a, &d, &b, &d, &c, &d);
			
				vec.push_back({ { a, b, c, d, 0 } });

			}

		}
	}

	float	xmin = 0.0;
	float	xmax = 0.0;
	float	ymin = 0.0;
	float	ymax = 0.0;
	float	zmin = 0.0;
	float	zmax = 0.0;

	Vector3f *normal;
	Vector3f *a;
	Vector3f *b;
	Vector3f *c;
	Triangle *triangle;

	
	for (int i = 0; i < vec.size(); i++){	

		a = vertex[(vec[i])[0] - 1];
		b = vertex[(vec[i])[1] - 1];
		c = vertex[(vec[i])[2] - 1];

		normal = normals[(vec[i])[3] - 1];

		xmin = min(a->getVec()[0], min(b->getVec()[0], min(c->getVec()[0], xmin)));
		ymin = min(a->getVec()[1], min(b->getVec()[1], min(c->getVec()[1], ymin)));
		zmin = min(a->getVec()[2], min(b->getVec()[2], min(c->getVec()[2], zmin)));

		xmax = max(a->getVec()[0], max(b->getVec()[0], max(c->getVec()[0], xmax)));
		ymax = max(a->getVec()[1], max(b->getVec()[1], max(c->getVec()[1], ymax)));
		zmax = max(a->getVec()[2], max(b->getVec()[2], max(c->getVec()[2], zmax)));

		
		triangle = new Triangle(*a , *b , *c , *normal);

		triangles.push_back(triangle);

	}
	
	Vector3f p1, p2;
	p1 = Vector3f(xmin, ymin, zmin), p2 = Vector3f(xmax, ymax, zmax);
	m_Extends = BBox(p1, p2 - p1);
	std::cout << "Build KDTree!" << std::endl;
	m_KdTree = new KdTree();
	m_KdTree->Build(triangles, m_Extends);
	std::cout << "Finished KDTree!" << std::endl;

	for (int i = 0; i < coord.size(); i++){
		delete coord[i];

	}
	
	for (int i = 0; i < normals.size(); i++){
		delete normals[i];
	}
	for (int i = 0; i < vertex.size(); i++){
		delete vertex[i];
	}


	return true;
}








