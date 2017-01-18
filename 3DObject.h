///////////////////////////////////////////////////////////////////////
// 3DObject.h

#pragma once

#define		PT_CCW_EX		0x08
#define		PT_STOP			0x10

typedef	pair<size_t, BYTE>	face_t;

class C3DProjector;

class C3DObject
{
public:
	C3DObject();
	virtual ~C3DObject();

	virtual void Clear();
	virtual void Draw(C3DProjector* pProjector) = 0;
	virtual bool IsVisible(C3DProjector* pProjector);

	void RotateX(double lfDegree);
	
	void Move(const glm::dmat4& matTrans);
	void Move(const glm::dvec3& vecTrans);
	void MoveX(double lfTrans);
	void MoveZ(double lfTrans);

	void Scale(double lfFactor);

	inline void SetColor(double r, double g, double b)
	{
		m_colR	= r;
		m_colG	= g;
		m_colB	= b;
	}
	virtual glm::dvec3 GetBoundingBox() const
	{
		glm::dvec3 r(1);
		return r;

	}
	inline void SetAlpha(double value)
	{
		if (value >= 0.0 && value <= 1.0)
			m_alpha = value;
	}
	inline double GetAlpha() const
	{
		return m_alpha;
	}
public:
	vector<glm::dvec4>			m_vecVertices;
	double						m_colR;
	double						m_colG;
	double						m_colB;
protected:
	double						m_alpha;
};

