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
	// imgui stuff
	void InitImGui(HWND hWnd = NULL, bool bIniFile = false);
	void ShutdownImGui();
	void NewFrameImGui(POINT ptMouse);
	void RenderImGui();

private:
	static void ImGui_Impl_RenderDrawLists(ImDrawData* draw_data);
	virtual void RenderDrawLists(ImDrawData* draw_data);
	void generic_triangle_2d(float x0, float y0, float x1, float y1, float x2, float y2, float u0, float v0, float u1, float v1, float u2, float v2, float c0, float c1, float c2, float a0, float a1, float a2);
	void generic_triangle_2d_c(float x0, float y0, float x1, float y1, float x2, float y2, float u0, float v0, float u1, float v1, float u2, float v2, const ImColor& c0, const ImColor& c1, const ImColor& c2);
	void blend_color(int x, int y, float c, float a);
	void blend_color_c(int x, int y, float r, float g, float b, float a);

protected:
	SIZE				m_szViewport;
	double				m_lfWidth;
	double				m_lfHeight;
	double				m_centerH;
	double				m_centerV;
	UINT				m_nStride;

	glm::dvec4			m_c;	// cam 
	glm::dvec4			m_e;	// viewer
	
	unsigned char*		m_pImGuiFontTexture;
	int					m_nFontTextureWidth;
	int					m_nFontTextureHeight;
	LARGE_INTEGER		m_ImGuiPrevTime;
	LARGE_INTEGER		m_TicksPerSecond;

public:
	glm::dmat4			m_matCamView;

protected:
	unsigned char*		m_pCurrentFrameBuffer;

};

