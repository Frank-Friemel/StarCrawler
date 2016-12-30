#pragma once

#include "3DGlyph.h"

class C3DWord : public C3DObject
{
public:
	C3DWord();
	~C3DWord();

	bool Create(PCWSTR strWord, const LOGFONT* pLogFont, bool bHinted = false);

	virtual void Clear();
	virtual void Draw(C3DProjector* pProjector);

	glm::dvec3 GetBoundingBox() const;

	void RotateX(double lfDegree)
	{
		__super::RotateX(lfDegree);

		for each (auto& c in m_listChars)
			c->RotateX(lfDegree);
	}
	void Move(const glm::dmat4& matTrans)
	{
		__super::Move(matTrans);

		for each (auto& c in m_listChars)
			c->Move(matTrans);
	}
	void Move(const glm::dvec3& vecTrans)
	{
		__super::Move(vecTrans);

		for each (auto& c in m_listChars)
			c->Move(vecTrans);
	}
	void MoveX(double lfTrans)
	{
		__super::MoveX(lfTrans);

		for each (auto& c in m_listChars)
			c->MoveX(lfTrans);
	}
	void MoveZ(double lfTrans)
	{
		__super::MoveZ(lfTrans);

		for each (auto& c in m_listChars)
			c->MoveZ(lfTrans);
	}
	void Scale(double lfFactor)
	{
		__super::Scale(lfFactor);

		for each (auto& c in m_listChars)
			c->Scale(lfFactor);
	}
	void SetColor(double r, double g, double b)
	{
		__super::SetColor(r, g, b);

		for each (auto& c in m_listChars)
			c->SetColor(r, g, b);
	}
	void SetAlpha(double value)
	{
		if (value >= 0.0 && value <= 1.0)
		{
			__super::SetAlpha(value);

			for each (auto& c in m_listChars)
				c->SetAlpha(value);
		}
	}

private:
	std::list< std::shared_ptr<C3DGlyph> > 	m_listChars;
};

