//////////////////////////////////////////////////////////////
// 3DProjector.h

#pragma once

typedef struct
{
	double x;
	double y;
} PIXEL2D;

class C3DProjector
{
public:
	C3DProjector(double zCam = 10, double zViewer = 1200);
	~C3DProjector();

	void Prepare(const SIZE& sz);
	void VertexToPixel(const glm::dvec4& v, PIXEL2D& ptPixel) const;

	void PixelToVertexStaticX(double xStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticY(double yStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticZ(double zStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;

	virtual void PolyDraw(const PIXEL2D* lppt, const BYTE* lpbTypes, size_t n, double r, double g, double b, double alpha) = 0;

	// 0.0 .... 1.0
	static double	Random();

	inline bool	IsVisible(const glm::dvec4& pos)
	{
		PIXEL2D ptScreen;
		VertexToPixel(pos, ptScreen);

		return IsVisible(ptScreen);
	}
	inline bool	IsVisible(const PIXEL2D& ptScreen)
	{
		if (ptScreen.x >= 0.0 && ptScreen.x < m_lfWidth && ptScreen.y >= 0.0 && ptScreen.y < m_lfHeight)
		{
			return true;
		}
		return false;
	}
	inline double GetWidth() const
	{
		return m_lfWidth;
	}
	inline double GetHeight() const
	{
		return m_lfHeight;
	}
private:
	double				m_lfWidth;
	double				m_lfHeight;
	double				m_centerH;
	double				m_centerV;

	glm::dvec4			m_c;	// cam 
	glm::dvec4			m_e;	// viewer
		
public:
	glm::dmat4			m_matCamView;
};

