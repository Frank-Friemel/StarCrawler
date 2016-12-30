#include "stdafx.h"
#include "3DStar.h"
#include "3DProjector.h"

C3DStar::C3DStar()
{
}


C3DStar::~C3DStar()
{
}

void C3DStar::Clear()
{
	__super::Clear();
	m_vecFaces.clear();
}

void C3DStar::Draw(C3DProjector* pProjector)
{
	ATLASSERT(pProjector);

	vector<PIXEL2D>	vecPoints(m_vecVertices.size());

	size_t i = 0;

	bool bIsVisible = false;

	for each(auto& v in m_vecVertices)
	{
		pProjector->VertexToPixel(v, vecPoints[i]);

		if (!bIsVisible && pProjector->IsVisible(vecPoints[i]))
			bIsVisible = true;
		++i;
	}
	if (bIsVisible)
	{
		size_t n = m_vecFaces.size();

		vector<PIXEL2D>	vecPolyPoints(n);
		vector<BYTE>	vecPolyTypes(n);

		i = 0;

		for each(auto&f in m_vecFaces)
		{
			vecPolyPoints[i] = vecPoints[f.first];
			vecPolyTypes[i++] = f.second;
		}
		pProjector->PolyDraw(&vecPolyPoints.front(), &vecPolyTypes.front(), n, m_colR, m_colG, m_colB, m_alpha);

	}
}

void  C3DStar::Randomize(C3DProjector* pProjector, double lfMoveToDistance)
{
	Clear();

	static int nEdges		= 16;
	
	static double* sin_x	= NULL;
	static double* cos_y	= NULL;

	// this is not thread safe, but 3D Objects are modified by a single thread
	if (!sin_x)
	{
		// pre calc the coords of the edges
		double angle_step	= (2 * M_PI) / ((double)nEdges);
		
		sin_x = new double[nEdges -1];
		cos_y = new double[nEdges -1];

		for (int i = 1; i < nEdges; ++i)
		{
			double rad = angle_step * ((double)i);
			
			sin_x[i-1] = sin(rad)*1.1;
			cos_y[i-1] = cos(rad)-1.0;
		}
	}
	m_vecVertices.push_back(glm::dvec4(0, 0, 0, 1));
	m_vecFaces.push_back(face_t(0, PT_MOVETO));

	for (int i = 1; i < nEdges; ++i)
	{
		m_vecVertices.push_back(glm::dvec4(sin_x[i-1], cos_y[i-1], 0, 1));
		m_vecFaces.push_back(face_t(i, PT_LINETO));
	}
	m_vecFaces.push_back(face_t(0, PT_STOP));

	glm::dvec4 coord(0, 0, 0, 1);

	PIXEL2D ptScreen;

	ptScreen.x = 5.0 + (pProjector->GetWidth() - 10.0) * C3DProjector::Random();
	ptScreen.y = 5.0 + (pProjector->GetHeight() - 10.0) * C3DProjector::Random();

	pProjector->PixelToVertexStaticZ(lfMoveToDistance - 50.0*C3DProjector::Random(), coord, ptScreen);
	ATLASSERT(pProjector->IsVisible(coord));

	SetColor(0.7 + 0.3 * C3DProjector::Random(), 0.7 + 0.3 *C3DProjector::Random(), 0.7 + 0.3 * C3DProjector::Random());

	Scale(0.12 + (C3DProjector::Random() - 0.5) / 50.0);
	Move(coord);
}
