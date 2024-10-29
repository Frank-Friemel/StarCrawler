///////////////////////////////////////////////////////////////////////
// 3DModel.h

#pragma once

#include "3DObject.h"

#define PT_CCW_EX		0x08 // close polygon
#define PT_STOP			0x10 // stop marker

// first:  index of vertex
// second: path-type (PT_MOVETO, PT_LINETO, PT_CCW_EX, PT_BEZIERTO or PT_STOP)
typedef	std::pair<size_t, uint8_t>	face_t;

// 3D-Model wires the vertices
class C3DModel : public C3DObject
{
public:
	C3DModel(const char* strModelFile = NULL);
	virtual ~C3DModel();

	void Clear() override;
	void Draw(I3DProjector* pProjector, uint8_t* frameBuffer) const override;

protected:
	std::vector<face_t> m_vecFaces;
};

