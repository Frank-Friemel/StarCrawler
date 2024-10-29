#pragma once

#include "3DModel.h"

class C3DGlyph : public C3DModel
{
public:
	C3DGlyph();
	~C3DGlyph();

	bool Create(WCHAR c, const LOGFONT* pLogFont, bool bHinted = false);

	glm::dvec3 GetBoundingBox() const override;
};

