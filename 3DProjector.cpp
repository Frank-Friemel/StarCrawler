#include "stdafx.h"
#include "3DProjector.h"
#include "time.h"

C3DProjector::C3DProjector(double zCam /*= 10*/, double zViewer /*= 1200*/)
{
	m_lfWidth	= 0;
	m_lfHeight	= 0;
	m_centerH	= 0;
	m_centerV	= 0;

	m_c			= glm::dvec4(0.0, 0.0, zCam, 1);
	m_e			= glm::dvec4(0.0, 0.0, zViewer, 1);

	m_matCamView= glm::lookAt(glm::dvec3(0, 0, 1), glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0));

	srand((unsigned int)::time(NULL));
}

C3DProjector::~C3DProjector()
{
}

double	C3DProjector::Random()
{
	return ((double)::rand()) / ((double)RAND_MAX);	
}

void C3DProjector::Prepare(const SIZE& sz)
{
	m_lfWidth	= sz.cx;
	m_lfHeight	= sz.cy;
	m_centerH	= m_lfWidth / 2.0;
	m_centerV	= m_lfHeight / 2.0;
}

// 3D to 2D projection
void C3DProjector::VertexToPixel(const glm::dvec4& v, PIXEL2D& ptPixel) const
{
	glm::dvec4 r;
	glm::dvec4 d;

	r = m_matCamView * v;
	d = r - m_c;

	double k = 0;

	if (d.z)
		k = m_e.z / d.z;

	double x = (k * d.x) - m_e.x;
	double y = (k * d.y) - m_e.y;

	ptPixel.x = m_centerH - x;
	ptPixel.y = m_centerV - y;
}

void C3DProjector::PixelToVertexStaticX(double xStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.x = xStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _a*v.x + _d*v.w - m_c.x;
	double kk4 = _e*v.x + _h*v.w - m_c.y;
	double kk5 = _i*v.x + _l*v.w - m_c.z;

	double kk6 = kk3 - kk1*kk5;
	double kk7 = kk4 - kk2*kk5;
	double kk8 = _j*kk2 - _f;
	double kk9 = _k*kk1 - _c;
	double kk10 = (_g* - _k*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_b - _j*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.z = (kk12*kk11 + kk13) / (1 - kk12*kk10);
	v.y = kk10*v.z + kk11;
}

void C3DProjector::PixelToVertexStaticY(double yStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.y = yStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _d*v.w + _b*v.y - m_c.x;
	double kk4 = _h*v.w + _f*v.y - m_c.y;
	double kk5 = _l*v.w + _j*v.y - m_c.z;
	double kk6 = kk3 - kk5*kk1;
	double kk7 = kk4 - kk5*kk2;
	double kk8 = _k*kk2 - _g;
	double kk9 = _i*kk1 - _a;
	double kk10 = (_e - _i*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_c - _k*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.x = (kk11*kk12 + kk13) / (1 - kk10*kk12);
	v.z = v.x * kk10 + kk11;
}

void C3DProjector::PixelToVertexStaticZ(double zStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.z = zStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _c*v.z + _d*v.w - m_c.x;
	double kk4 = _g*v.z + _h*v.w - m_c.y;
	double kk5 = _k*v.z + _l*v.w - m_c.z;

	double kk6 = kk3 - kk1*kk5;
	double kk7 = kk4 - kk2*kk5;
	double kk8 = _j*kk2 - _f;
	double kk9 = _i*kk1 - _a;
	double kk10 = (_e* - _i*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_b - _j*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.x = (kk12*kk11 + kk13) / (1 - kk12*kk10);
	v.y = kk10*v.x  + kk11;
}



