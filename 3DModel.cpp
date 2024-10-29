#include "stdafx.h"
#include "3DModel.h"
#include <fstream>
#include <sstream>

using namespace std;

C3DModel::C3DModel(const char* strModelFile /*= NULL*/)
{
	if (strModelFile)
	{
		std::ifstream in;

		in.open(strModelFile, std::ifstream::in);
	
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

void C3DModel::Draw(I3DProjector* pProjector, uint8_t* frameBuffer) const
{
	ATLASSERT(pProjector);
	ATLASSERT(frameBuffer);

	bool bIsVisible = false;

	const size_t count = m_vecFaces.size();

	vector<PolyPoint> vecPolyPoints(count);

	auto polyPoint = vecPolyPoints.data();

	for (const auto& f : m_vecFaces)
	{
		if (pProjector->IsVisible(m_vecVertices[f.first], &polyPoint->pixel))
		{
			bIsVisible = true;
		}
		polyPoint->pathType = f.second;
		++polyPoint;
	}
	if (bIsVisible)
	{
		pProjector->PolyDraw(frameBuffer, vecPolyPoints.data(), count, m_colR, m_colG, m_colB, m_alpha);
	}
}
