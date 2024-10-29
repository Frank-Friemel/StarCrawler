#pragma once

#include <glm/fwd.hpp>
#include <stdint.h>

typedef struct
{
	double x;
	double y;
} PIXEL2D;

typedef struct
{
	PIXEL2D pixel;
	uint8_t	pathType; // path-type (PT_MOVETO, PT_LINETO, PT_CCW_EX, PT_BEZIERTO or PT_STOP)
} PolyPoint;

class I3DProjector
{
public:
	virtual ~I3DProjector() = default;

	virtual void	PolyDraw(uint8_t* frameBuffer, const PolyPoint* polyPoints, size_t count, double r, double g, double b, double alpha) = 0;
	virtual void	VertexToPixel(const glm::dvec4& vertex, PIXEL2D& ptPixel) const noexcept  = 0;
	virtual bool	IsVisible(const glm::dvec4& vertex, PIXEL2D* ptPixel = nullptr) const noexcept = 0;
};