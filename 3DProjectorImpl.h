//////////////////////////////////////////////////////////////
// 3DProjectorImpl.h

#pragma once

#pragma once

#include "imgui\imgui.h"
#include "I3DProjector.h"

class C3DProjectorImpl 
	: public I3DProjector
{
public:
	C3DProjectorImpl(double zCam = 10, double zViewer = 1200);
	~C3DProjectorImpl();

	void Prepare(const SIZE& sz, UINT nBitplaneCount);
	
	// 3D to 2D projection
	void	VertexToPixel(const glm::dvec4& vertex, PIXEL2D& ptPixel) const noexcept override;
	bool	IsVisible(const glm::dvec4& vertex, PIXEL2D* ptPixel = nullptr) const noexcept override;

protected:
	// 2D to 3D projection
	void PixelToVertexStaticX(double xStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticY(double yStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;
	void PixelToVertexStaticZ(double zStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const;

	double	GetWidth() const;
	double	GetHeight() const;

	// 0.0 .... 1.0
	static double Random();

	// imgui stuff
	void InitImGui(HWND hWnd = NULL, bool bIniFile = false);
	void ShutdownImGui();
	void NewFrameImGui(uint8_t* frameBuffer);
	void RenderImGui();

	BEGIN_MSG_MAP_EX(C3DProjectorImpl)
		switch (uMsg)
		{
			case WM_LBUTTONDOWN:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[0] = true;
			}
			break;
			
			case WM_LBUTTONDBLCLK:
			{
				++m_bDoubleClick;
			}
			break;
			case WM_LBUTTONUP:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[0] = false;
			}
			break;
			case WM_RBUTTONDOWN:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[1] = true;
			}
			break;
			case WM_RBUTTONUP:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[1] = false;
			}
			break;
			case WM_MBUTTONDOWN:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[2] = true;
			}
			break;
			case WM_MBUTTONUP:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[2] = false;
			}
			break;
			case WM_MOUSEWHEEL:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			}
			break;
			case WM_MOUSEMOVE:
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MousePos.x = (signed short)(lParam);
				io.MousePos.y = (signed short)(lParam >> 16);
			}
			break;
			case WM_KEYDOWN:
			{
				if (wParam < 256)
				{
					ImGuiIO& io = ImGui::GetIO();
					io.KeysDown[wParam] = 1;
				}			
			}
			break;
			case WM_KEYUP:
			{
				if (wParam < 256)
				{
					ImGuiIO& io = ImGui::GetIO();
					io.KeysDown[wParam] = 0;
				}			
			}
			break;
			case WM_CHAR:
			{
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
				{	
					ImGuiIO& io = ImGui::GetIO();
					io.AddInputCharacter((unsigned short)wParam);
				}
			}
			break;
		}
	END_MSG_MAP()

protected:
	// ImGui stuff
	virtual void	BlendColor(uint8_t * frameBuffer, int x, int y, int r, int g, int b, int a);
	virtual void	SetScissors(int left, int top, int right, int bottom);
	virtual void	ClearScissors();
	virtual void	RenderDrawLists(ImDrawData* draw_data);

private:
	static void		ImGui_Impl_RenderDrawLists(ImDrawData* draw_data);
	bool			TriangleDraw(uint8_t* frameBuffer, const ImDrawVert& vertex0, const ImDrawVert& vertex1,
							const ImDrawVert& vertex2, const float* pTexture, int nTextureWidth, int nTextureHeight);

protected:
	SIZE				m_szViewport;
	double				m_lfWidth;
	double				m_lfHeight;
	double				m_centerH;
	double				m_centerV;
	UINT				m_nStride;

	const glm::dvec4	m_cameraPosition;	// camera position 
	const glm::dvec4	m_viewerPosition;	// viewer position
	
	CTempBuffer<float>	m_pImGuiTexture;
	int					m_nTextureWidth;
	int					m_nTextureHeight;
	LARGE_INTEGER		m_ImGuiPrevTime;
	float				m_TicksPerSecond;
	CRect				m_rectClip;
	bool				m_bClip;

protected:
	glm::dmat4			m_matCamView;

private:
	std::atomic_int		m_bDoubleClick{ 0 };
};

