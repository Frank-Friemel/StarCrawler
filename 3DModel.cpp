#include "stdafx.h"
#include "3DModel.h"
#include "3DProjector.h"
#include <fstream>
#include <sstream>

C3DModel::C3DModel(PCWSTR strModelFile /*= NULL*/)
{
	if (strModelFile)
	{
		USES_CONVERSION;

		std::ifstream in;

		in.open(W2CA(strModelFile), std::ifstream::in);
	
		if (in.good())
		{
			std::string line;
	
			while (!in.eof()) 
			{
				std::getline(in, line);
				std::istringstream iss(line.c_str());
				
				char trash;
				
				if (!line.compare(0, 2, "v ")) 
				{
					iss >> trash;
					glm::dvec4 v;
					
					iss >> v.x;
					v.x *= (-1.0);

					iss >> v.y;
					v.y *= (-1.0);

					iss >> v.z;
					//v.z *= (-1.0);

					v.w = 1;

					m_vecVertices.push_back(v);
				} 
				else if (!line.compare(0, 2, "f "))
				{
					int itrash, idx;
					iss >> trash;
					
					while (iss >> idx >> trash >> itrash >> trash >> itrash) 
					{
						idx--;
						m_vecFaces.push_back(face_t(idx, m_vecFaces.empty() ? PT_MOVETO : PT_LINETO));
					}
				}
			}
		}
	}
}

C3DModel::~C3DModel()
{
}

void C3DModel::Clear()
{
	__super::Clear();
	m_vecFaces.clear();
}

void C3DModel::Draw(C3DProjector* pProjector)
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
