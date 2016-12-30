#pragma once

#include "3DObject.h"

class C3DGlyph : public C3DObject
{
public:
	C3DGlyph();
	~C3DGlyph();

	bool Create(WCHAR c, const LOGFONT* pLogFont, bool bHinted = false);

	virtual void Clear();
	virtual void Draw(C3DProjector* pProjector);

	glm::dvec3 GetBoundingBox() const;

private:
	vector<face_t>				m_vecFaces;
};

