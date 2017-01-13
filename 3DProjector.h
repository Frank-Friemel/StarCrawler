//////////////////////////////////////////////////////////////
// 3DProjector.h

#pragma once

#include "imgui\imgui.h"

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

	void Prepare(const SIZE& sz, UINT nBitplaneCount);
	void VertexToPixel(const glm::dvec4& v, PIXEL2D& ptPixel) const;

	void PixelToVertexStaticX(double xStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticY(double yStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticZ(double zStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;

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
protected:
	// imgui stuff
	void InitImGui(HWND hWnd = NULL, bool bIniFile = false);
	void ShutdownImGui();
	void NewFrameImGui();
	void RenderImGui();

public:
	// implementation
	virtual void	PolyDraw(const PIXEL2D* lppt, const BYTE* lpbTypes, size_t n, double r, double g, double b, double alpha) = 0;
	virtual void	BlendColor(int x, int y, int r, int g, int b, int a);
	virtual void	SetScissors(int left, int top, int right, int bottom)
	{
		m_rectClip.left		= left;
		m_rectClip.top		= top;
		m_rectClip.right	= right;
		m_rectClip.bottom	= bottom;
		m_bClip				= true;
	}
	virtual void	ClearScissors()
	{
		m_bClip = false;
	}
protected:
	virtual void	RenderDrawLists(ImDrawData* draw_data);

private:
	static void		ImGui_Impl_RenderDrawLists(ImDrawData* draw_data);
	bool			TriangleDraw(const ImDrawVert& vertex0, const ImDrawVert& vertex1, const ImDrawVert& vertex2);

protected:
	SIZE				m_szViewport;
	double				m_lfWidth;
	double				m_lfHeight;
	double				m_centerH;
	double				m_centerV;
	UINT				m_nStride;

	glm::dvec4			m_c;	// cam 
	glm::dvec4			m_e;	// viewer
	
	unsigned char*		m_pImGuiTexture;
	int					m_nTextureWidth;
	int					m_nTextureHeight;
	LARGE_INTEGER		m_ImGuiPrevTime;
	float				m_TicksPerSecond;
	CRect				m_rectClip;
	bool				m_bClip;

public:
	glm::dmat4			m_matCamView;

protected:
	unsigned char*		m_pCurrentFrameBuffer;

};

