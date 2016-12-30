#include "stdafx.h"
#include "3DObject.h"
#include "3DProjector.h"

C3DObject::C3DObject()
{
	m_colR	= 0.0;
	m_colG	= 1.0;
	m_colB	= 0.0;
	m_alpha	= 1.0;
}


C3DObject::~C3DObject()
{
}

void C3DObject::Clear()
{
	m_vecVertices.clear();
}

bool C3DObject::IsVisible(C3DProjector* pProjector)
{
	ATLASSERT(pProjector);

	vector<PIXEL2D>	vecPoints(m_vecVertices.size());

	size_t i = 0;

	bool bIsVisible = false;

	for each(auto& v in m_vecVertices)
	{
		pProjector->VertexToPixel(v, vecPoints[i]);

		if (!bIsVisible && pProjector->IsVisible(vecPoints[i]))
		{ 
			bIsVisible = true;
			break;
		}
		++i;
	}
	return bIsVisible;
}

void C3DObject::RotateX(double lfDegree)
{
	glm::dvec3 myRotationAxis(1, 0, 0);

	glm::dmat4 c = glm::rotate((glm::dvec3::value_type)glm::radians(lfDegree), myRotationAxis);

	for (auto v = m_vecVertices.begin(); v != m_vecVertices.end(); ++v)
		*v = c * (*v);
}

void C3DObject::MoveX(double lfTrans)
{
	glm::dvec3 vecTrans(lfTrans, 0, 0);
	Move(vecTrans);
}

void C3DObject::MoveZ(double lfTrans)
{
	glm::dvec3 vecTrans(0, 0, lfTrans);
	Move(vecTrans);
}

void C3DObject::Move(const glm::dvec3& vecTrans)
{
	glm::dmat4 c = glm::translate(vecTrans);

	for (auto v = m_vecVertices.begin(); v != m_vecVertices.end(); ++v)
		*v = c * (*v);
}

void C3DObject::Move(const glm::dmat4& matTrans)
{
	for (auto v = m_vecVertices.begin(); v != m_vecVertices.end(); ++v)
		*v = matTrans * (*v);
}

void C3DObject::Scale(double lfFactor)
{
	glm::mat4 matScaling = glm::scale(glm::dvec3(lfFactor, lfFactor, lfFactor));

	for (auto v = m_vecVertices.begin(); v != m_vecVertices.end(); ++v)
		*v = matScaling * (*v);
}