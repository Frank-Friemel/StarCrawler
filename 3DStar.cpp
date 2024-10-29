#include "stdafx.h"
#include "3DStar.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif

using namespace std;

constexpr int nEdges = 16;

class Circle
{
public:
	Circle()
	{
		vector<double> sin_x;
		vector<double> cos_y;

		sin_x.resize(nEdges - 1);
		cos_y.resize(nEdges - 1);

		const double angle_step = (2 * M_PI) / ((double)nEdges);

		for (int i = 1; i < nEdges; ++i)
		{
			double rad = angle_step * ((double)i);

			sin_x[i - 1] = sin(rad) * 1.1;
			cos_y[i - 1] = cos(rad) - 1.0;
		}

		m_vecVertices.push_back(glm::dvec4(0, 0, 0, 1));
		m_vecFaces.push_back(face_t(0, PT_MOVETO));

		for (int i = 1; i < nEdges; ++i)
		{
			m_vecVertices.push_back(glm::dvec4(sin_x[i - 1], cos_y[i - 1], 0, 1));
			m_vecFaces.push_back(face_t(i, PT_LINETO));
		}
		m_vecFaces.push_back(face_t(0, PT_STOP));
	}

public:
	vector<glm::dvec4> m_vecVertices;
	vector<face_t> m_vecFaces;
};

static const Circle circle;

C3DStar::C3DStar()
{
}

C3DStar::~C3DStar()
{
}

void C3DStar::InitNew()
{
	m_vecVertices = circle.m_vecVertices;
	m_vecFaces = circle.m_vecFaces;
}
