#pragma once

#include "3DGlyph.h"

class C3DWord : public C3DObject
{
public:
	C3DWord();
	~C3DWord();

	bool Create(PCWSTR strWord, const LOGFONT* pLogFont, bool bHinted = false);

	void Clear() override;
	void Draw(I3DProjector* pProjector, uint8_t* frameBuffer) const override;
	glm::dvec3 GetBoundingBox() const override;

	void RotateX(double lfDegree);
	void Move(const glm::dmat4& matTrans);
	void Move(const glm::dvec3& vecTrans);
	void MoveX(double lfTrans);
	void MoveZ(double lfTrans);
	void Scale(double lfFactor);
	void SetColor(double r, double g, double b);
	void SetAlpha(double value);

private:
	std::list<std::shared_ptr<C3DGlyph>> m_listChars;
};

