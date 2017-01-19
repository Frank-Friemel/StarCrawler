#pragma once

#include "3DModel.h"

class C3DStar : public C3DModel
{
public:
	C3DStar();
	~C3DStar();

	void Randomize(C3DProjector* pProjector, double lfMoveToDistance);

};

