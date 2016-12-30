#pragma once

#include "3DObject.h"

class C3DStar : public C3DObject
{
public:
	C3DStar();
	~C3DStar();

	virtual void Clear();
	virtual void Draw(C3DProjector* pProjector);

	void Randomize(C3DProjector* pProjector, double lfMoveToDistance);

private:
	vector<face_t>				m_vecFaces;
};

