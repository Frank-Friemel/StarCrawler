///////////////////////////////////////////////////////////////////////
// 3DModel.h

#pragma once


#include "3DObject.h"


class C3DModel : public C3DObject
{
public:
	C3DModel(PCWSTR strModelFile = NULL);
	virtual ~C3DModel();

	virtual void Clear();
	virtual void Draw(C3DProjector* pProjector);

protected:
	vector<face_t>				m_vecFaces;
};

