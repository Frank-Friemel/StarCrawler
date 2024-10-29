///////////////////////////////////////////////////////////////////////
// 3DObject.h

#pragma once

#include "I3DProjector.h"

// 3D-Object is basically a collection of vertices with a RGB-color and an alpha-channel
class C3DObject
{
public:
	C3DObject();
	virtual ~C3DObject();

	virtual void Clear();
	virtual void Draw(I3DProjector* pProjector, uint8_t* frameBuffer) const = 0;
	virtual bool IsVisible(I3DProjector* pProjector) const noexcept;
	virtual glm::dvec3 GetBoundingBox() const;

	void RotateX(double lfDegree);
	
	void Move(const glm::dmat4& matTrans);
	void Move(const glm::dvec3& vecTrans);
	void MoveX(double lfTrans);
	void MoveZ(double lfTrans);

	void Scale(double lfFactor);

	inline void SetColor(double r, double g, double b) noexcept
	{
		m_colR	= r;
		m_colG	= g;
		m_colB	= b;
	}
	inline void SetAlpha(double value)
	{
		if (value >= 0.0 && value <= 1.0)
		{
			m_alpha = value;
		}
	}
	inline double GetAlpha() const
	{
		return m_alpha;
	}

public:
	std::vector<glm::dvec4>		m_vecVertices; // vertex (a collection of vertices)
	double						m_colR;
	double						m_colG;
	double						m_colB;

protected:
	double						m_alpha;
};

