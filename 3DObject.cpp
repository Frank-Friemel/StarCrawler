#include "stdafx.h"
#include "3DObject.h"

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

bool C3DObject::IsVisible(I3DProjector* pProjector) const noexcept
{
	ATLASSERT(pProjector);

	for (const auto& v : m_vecVertices)
	{
		if (pProjector->IsVisible(v))
		{ 
			return true;
		}
	}
	return false;
}

glm::dvec3 C3DObject::GetBoundingBox() const
{
	const glm::dvec3 r(1);
	return r;
}

void C3DObject::RotateX(double lfDegree)
{
	const glm::dvec3 myRotationAxis(1, 0, 0);
	const glm::dmat4 c = glm::rotate((glm::dvec3::value_type)glm::radians(lfDegree), myRotationAxis);

	for (auto& v : m_vecVertices)
	{
		v = c * v;
	}
}

void C3DObject::MoveX(double lfTrans)
{
	const glm::dvec3 vecTrans(lfTrans, 0, 0);
	Move(vecTrans);
}

void C3DObject::MoveZ(double lfTrans)
{
	const glm::dvec3 vecTrans(0, 0, lfTrans);
	Move(vecTrans);
}

void C3DObject::Move(const glm::dvec3& vecTrans)
{
	const glm::dmat4 c = glm::translate(vecTrans);

	for (auto& v : m_vecVertices)
	{
		v = c * v;
	}
}

void C3DObject::Move(const glm::dmat4& matTrans)
{
	for (auto& v : m_vecVertices)
	{
		v = matTrans * v;
	}
}

void C3DObject::Scale(double lfFactor)
{
	const glm::mat4 matScaling = glm::scale(glm::dvec3(lfFactor, lfFactor, lfFactor));

	for (auto& v : m_vecVertices)
	{
		v = matScaling * v;
	}
}