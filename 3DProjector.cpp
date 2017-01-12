#include "stdafx.h"
#include "3DObject.h"
#include "3DProjector.h"
#include "time.h"
#include "imgui/imgui.h"

#define mymax(__a,__b) ((__a) > (__b) ? (__a) : (__b))
#define mymin(__a,__b) ((__a) < (__b) ? (__a) : (__b))


typedef union _pixel_t
{
	_pixel_t(uint32_t i = 0)
	{
		integer = i;
	}
	_pixel_t( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0 )
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	uint32_t integer;			///< you can work directly with the truecolor integer if you like, but be careful about endianness!

	struct
	{
		uint8_t b;			///< blue component
		uint8_t g;			///< green component
		uint8_t r;			///< red component
		uint8_t a;			///< alpha component
	};
} pixel_t;


pixel_t color_to_pixel(float c)
{
	auto brightness = mymax(0, mymin(255, (int)(255 * c)));
	return pixel_t(brightness, brightness, brightness, 255);
}

C3DProjector::C3DProjector(double zCam /*= 10*/, double zViewer /*= 1200*/)
{
	m_lfWidth	= 0;
	m_lfHeight	= 0;
	m_centerH	= 0;
	m_centerV	= 0;
	m_nStride	= 0;

	m_c			= glm::dvec4(0.0, 0.0, zCam, 1);
	m_e			= glm::dvec4(0.0, 0.0, zViewer, 1);

	m_matCamView= glm::lookAt(glm::dvec3(0, 0, 1), glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0));

	srand((unsigned int)::time(NULL));
}

C3DProjector::~C3DProjector()
{
}

double	C3DProjector::Random()
{
	return ((double)::rand()) / ((double)RAND_MAX);	
}

void C3DProjector::Prepare(const SIZE& sz, UINT nBitplaneCount)
{
	m_szViewport= sz;

	m_lfWidth	= sz.cx;
	m_lfHeight	= sz.cy;
	m_centerH	= m_lfWidth / 2.0;
	m_centerV	= m_lfHeight / 2.0;

	m_nStride	= sz.cx * nBitplaneCount;
}

// 3D to 2D projection
void C3DProjector::VertexToPixel(const glm::dvec4& v, PIXEL2D& ptPixel) const
{
	glm::dvec4 r;
	glm::dvec4 d;

	r = m_matCamView * v;
	d = r - m_c;

	double k = 0;

	if (d.z)
		k = m_e.z / d.z;

	double x = (k * d.x) - m_e.x;
	double y = (k * d.y) - m_e.y;

	ptPixel.x = m_centerH - x;
	ptPixel.y = m_centerV - y;
}

void C3DProjector::PixelToVertexStaticX(double xStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.x = xStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _a*v.x + _d*v.w - m_c.x;
	double kk4 = _e*v.x + _h*v.w - m_c.y;
	double kk5 = _i*v.x + _l*v.w - m_c.z;

	double kk6 = kk3 - kk1*kk5;
	double kk7 = kk4 - kk2*kk5;
	double kk8 = _j*kk2 - _f;
	double kk9 = _k*kk1 - _c;
	double kk10 = (_g* - _k*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_b - _j*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.z = (kk12*kk11 + kk13) / (1 - kk12*kk10);
	v.y = kk10*v.z + kk11;
}

void C3DProjector::PixelToVertexStaticY(double yStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.y = yStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _d*v.w + _b*v.y - m_c.x;
	double kk4 = _h*v.w + _f*v.y - m_c.y;
	double kk5 = _l*v.w + _j*v.y - m_c.z;
	double kk6 = kk3 - kk5*kk1;
	double kk7 = kk4 - kk5*kk2;
	double kk8 = _k*kk2 - _g;
	double kk9 = _i*kk1 - _a;
	double kk10 = (_e - _i*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_c - _k*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.x = (kk11*kk12 + kk13) / (1 - kk10*kk12);
	v.z = v.x * kk10 + kk11;
}

void C3DProjector::PixelToVertexStaticZ(double zStatic, glm::dvec4& v, const PIXEL2D& ptPixel) const
{
	const glm::dvec4& col0 = m_matCamView[0];
	const glm::dvec4& col1 = m_matCamView[1];
	const glm::dvec4& col2 = m_matCamView[2];
	const glm::dvec4& col3 = m_matCamView[3];

	const double& _a = col0.x;
	const double& _e = col0.y;
	const double& _i = col0.z;

	const double& _b = col1.x;
	const double& _f = col1.y;
	const double& _j = col1.z;

	const double& _c = col2.x;
	const double& _g = col2.y;
	const double& _k = col2.z;

	const double& _d = col3.x;
	const double& _h = col3.y;
	const double& _l = col3.z;

	// set *one* static value (v.x, v.y or v.z)
	v.z = zStatic;
	v.w = 1;

	double kk1 = (m_centerH - ptPixel.x + m_e.x) / m_e.z;
	double kk2 = (m_centerV - ptPixel.y + m_e.y) / m_e.z;

	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk1 = (_a*v.x + _b*v.y + _c*v.z + _d*v.w - m_c.x); 
	// (_i*v.x + _j*v.y + _k*v.z + _l*v.w - m_c.z) * kk2 = (_e*v.x + _f*v.y + _g*v.z + _h*v.w - m_c.y); 

	double kk3 = _c*v.z + _d*v.w - m_c.x;
	double kk4 = _g*v.z + _h*v.w - m_c.y;
	double kk5 = _k*v.z + _l*v.w - m_c.z;

	double kk6 = kk3 - kk1*kk5;
	double kk7 = kk4 - kk2*kk5;
	double kk8 = _j*kk2 - _f;
	double kk9 = _i*kk1 - _a;
	double kk10 = (_e* - _i*kk2) / kk8;
	double kk11 = kk7 / kk8;
	double kk12 = (_b - _j*kk1) / kk9;
	double kk13 = kk6 / kk9;

	v.x = (kk12*kk11 + kk13) / (1 - kk12*kk10);
	v.y = kk10*v.x  + kk11;
}


// imgui stuff
void C3DProjector::InitImGui(HWND hWnd /* = NULL*/, bool bIniFile /*= false*/)
{
	ImGuiIO& io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_Tab]			= VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow]	= VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow]	= VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow]		= VK_UP;
	io.KeyMap[ImGuiKey_DownArrow]	= VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp]		= VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown]	= VK_NEXT;
	io.KeyMap[ImGuiKey_Home]		= VK_HOME;
	io.KeyMap[ImGuiKey_End]			= VK_END;
	io.KeyMap[ImGuiKey_Delete]		= VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace]	= VK_BACK;
	io.KeyMap[ImGuiKey_Enter]		= VK_RETURN;
	io.KeyMap[ImGuiKey_Escape]		= VK_ESCAPE;
	io.KeyMap[ImGuiKey_A]			= 'A';
	io.KeyMap[ImGuiKey_C]			= 'C';
	io.KeyMap[ImGuiKey_V]			= 'V';
	io.KeyMap[ImGuiKey_X]			= 'X';
	io.KeyMap[ImGuiKey_Y]			= 'Y';
	io.KeyMap[ImGuiKey_Z]			= 'Z';
	io.ImeWindowHandle				= hWnd;
	io.UserData						= this;
	io.RenderDrawListsFn			= ImGui_Impl_RenderDrawLists; 

	if (!bIniFile)
		io.IniFilename				= NULL;

	// Build texture atlas
	io.Fonts->GetTexDataAsAlpha8(&m_pImGuiFontTexture, &m_nFontTextureWidth, &m_nFontTextureHeight);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)&m_pImGuiFontTexture;

	QueryPerformanceCounter(&m_ImGuiPrevTime);
	QueryPerformanceFrequency(&m_TicksPerSecond);
}

void C3DProjector::ShutdownImGui()
{
	ImGui::Shutdown();
}

void C3DProjector::RenderImGui()
{
	ImGui::Render();
}

void C3DProjector::NewFrameImGui(POINT ptMouse)
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	io.DisplaySize				= ImVec2((float)m_lfWidth, (float)m_lfHeight);
	io.DisplayFramebufferScale	= ImVec2(1, 1);

	// Setup time step
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	io.DeltaTime = (float)(current_time.QuadPart - m_ImGuiPrevTime.QuadPart) / m_TicksPerSecond.QuadPart;
	m_ImGuiPrevTime = current_time;

	// Start the frame
	ImGui::NewFrame();
}

void C3DProjector::ImGui_Impl_RenderDrawLists(ImDrawData* draw_data)
{
	ImGuiIO& io = ImGui::GetIO();

	C3DProjector* pProjector = (C3DProjector*) io.UserData;

	pProjector->RenderDrawLists(draw_data);
}

void C3DProjector::RenderDrawLists(ImDrawData* draw_data)
{
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* const cmd_list = draw_data->CmdLists[n];

		auto& vertices = cmd_list->VtxBuffer;
		auto& indices  = cmd_list->IdxBuffer;

		int idx_offset = 0;

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else if (pcmd->ElemCount > 0)
			{
				auto* const indexStart = indices.Data + idx_offset;

				for (unsigned int i = 0; i < pcmd->ElemCount; i += 3)
				{
					const auto i0 = indexStart[i + 0];
					const auto i1 = indexStart[i + 1];
					const auto i2 = indexStart[i + 2];

					const auto v0 = vertices[i0];
					const auto v1 = vertices[i1];
					const auto v2 = vertices[i2];

					const auto vc0 = ImColor(v0.col);
					const auto vc1 = ImColor(v1.col);
					const auto vc2 = ImColor(v2.col);

					const auto c0 = (vc0.Value.x + vc0.Value.y + vc0.Value.z) / 3;
					const auto c1 = (vc1.Value.x + vc1.Value.y + vc1.Value.z) / 3;
					const auto c2 = (vc2.Value.x + vc2.Value.y + vc2.Value.z) / 3;

					if (pcmd->TextureId)
					{
						generic_triangle_2d_c(v0.pos.x, v0.pos.y,
											v2.pos.x, v2.pos.y,
											v1.pos.x, v1.pos.y,
											v0.uv.x, v0.uv.y,
											v2.uv.x, v2.uv.y,
											v1.uv.x, v1.uv.y,
											vc0, vc2, vc1);					
					}
					else
					{
						PIXEL2D pxl[4];
						BYTE	cmd[4];

						pxl[0].x	= v0.pos.x;
						pxl[0].y	= v0.pos.y;
						cmd[0]		= PT_MOVETO;

						pxl[1].x	= v1.pos.x+1;
						pxl[1].y	= v1.pos.y+1;
						cmd[1]		= PT_LINETO;

						pxl[2].x	= v2.pos.x+1;
						pxl[2].y	= v2.pos.y+1;
						cmd[2]		= PT_LINETO;

						pxl[3].x	= v0.pos.x+1;
						pxl[3].y	= v0.pos.y+1;
						cmd[3]		= PT_LINETO;

						PolyDraw(pxl, cmd, 4, vc0.Value.x, vc0.Value.y, vc0.Value.z, vc0.Value.w);
					}
				}
				//fill_rect_2d(*imgui_render_target, (int)min.x, (int)min.y, (int)max.x, (int)max.y, 1.0f);
				//const D3D10_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				//ctx->PSSetShaderResources(0, 1, (ID3D10ShaderResourceView**)&pcmd->TextureId);
				//ctx->RSSetScissorRects(1, &r);
				//ctx->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
	}
}


void C3DProjector::generic_triangle_2d(float x0, float y0, float x1, float y1, float x2, float y2, float u0, float v0, float u1, float v1, float u2, float v2, float c0, float c1, float c2, float a0, float a1, float a2)
{
	// 24.8 fixed-point
	const int precission = 4;
	const int mask       = (1 << precission) - 1;

	// Fixed-point coordinates
	const int Y0 = (int)(y0 * static_cast<float>(1 << precission));
	const int Y1 = (int)(y1 * static_cast<float>(1 << precission));
	const int Y2 = (int)(y2 * static_cast<float>(1 << precission));

	const int X0 = (int)(x0 * static_cast<float>(1 << precission));
	const int X1 = (int)(x1 * static_cast<float>(1 << precission));
	const int X2 = (int)(x2 * static_cast<float>(1 << precission));

	// Deltas
	const int DX01 = X0 - X1;
	const int DX12 = X1 - X2;
	const int DX20 = X2 - X0;

	const int DY01 = Y0 - Y1;
	const int DY12 = Y1 - Y2;
	const int DY20 = Y2 - Y0;

	// Fixed-point deltas
	const int FDX01 = DX01 << precission;
	const int FDX12 = DX12 << precission;
	const int FDX20 = DX20 << precission;

	const int FDY01 = DY01 << precission;
	const int FDY12 = DY12 << precission;
	const int FDY20 = DY20 << precission;

	// Bounding rectangle
	int minx = (mymin(mymin(X0, X1), X2) + mask) >> precission;
	int maxx = (mymax(mymax(X0, X1), X2) + mask) >> precission;
	int miny = (mymin(mymin(Y0, Y1), Y2) + mask) >> precission;
	int maxy = (mymax(mymax(Y0, Y1), Y2) + mask) >> precission;

	// Half-edge constants
	int C0 = DY01 * X0 - DX01 * Y0;
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY20 * X2 - DX20 * Y2;

	// Correct for fill convention
	if (DY01 < 0 || (DY01 == 0 && DX01 > 0)) C0++;
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if (DY20 < 0 || (DY20 == 0 && DX20 > 0)) C2++;

	int CY0 = C0 + DX01 * (miny << precission) - DY01 * (minx << precission);
	int CY1 = C1 + DX12 * (miny << precission) - DY12 * (minx << precission);
	int CY2 = C2 + DX20 * (miny << precission) - DY20 * (minx << precission);

	auto iC = 1.0f / (CY0 + CY1 + CY2);

	for (int y = miny; y < maxy; y++)
	{
		if (y >= 0 && y < m_szViewport.cy)
		{
			int CX0 = CY0;
			int CX1 = CY1;
			int CX2 = CY2;

			for (int x = minx; x < maxx; x++)
			{
				if (x >= 0 && x < m_szViewport.cx)
				{
					if (CX0 > 0 && CX1 > 0 && CX2 > 0)
					{
						const auto c = (c2 * CX0 + c0 * CX1 + c1 * CX2) * iC;
						const auto a = (a2 * CX0 + a0 * CX1 + a1 * CX2) * iC;
						const auto u = (u2 * CX0 + u0 * CX1 + u1 * CX2) * iC;
						const auto v = (v2 * CX0 + v0 * CX1 + v1 * CX2) * iC;

						const auto tx = mymax(0, mymin(static_cast<int>(u * (m_nFontTextureWidth)), m_nFontTextureWidth - 1));
						const auto ty = mymax(0, mymin(static_cast<int>(v * (m_nFontTextureHeight)), m_nFontTextureHeight - 1));

						const auto texture_color = m_pImGuiFontTexture[tx + ty * m_nFontTextureWidth] * (1.0f / 255.0f);

						blend_color(x, y, c, texture_color * a);
					}
				}
				else if (x >= m_szViewport.cx)
					break;

				CX0 -= FDY01;
				CX1 -= FDY12;
				CX2 -= FDY20;
			}
		}
		else if (y >= m_szViewport.cy)
			return;

		CY0 += FDX01;
		CY1 += FDX12;
		CY2 += FDX20;
	}
}

void C3DProjector::generic_triangle_2d_c(float x0, float y0, float x1, float y1, float x2, float y2, float u0, float v0, float u1, float v1, float u2, float v2, const ImColor& c0, const ImColor& c1, const ImColor& c2)
{
	// 24.8 fixed-point
	const int precission = 4;
	const int mask       = (1 << precission) - 1;

	// Fixed-point coordinates
	const int Y0 = (int)(y0 * static_cast<float>(1 << precission));
	const int Y1 = (int)(y1 * static_cast<float>(1 << precission));
	const int Y2 = (int)(y2 * static_cast<float>(1 << precission));

	const int X0 = (int)(x0 * static_cast<float>(1 << precission));
	const int X1 = (int)(x1 * static_cast<float>(1 << precission));
	const int X2 = (int)(x2 * static_cast<float>(1 << precission));

	// Deltas
	const int DX01 = X0 - X1;
	const int DX12 = X1 - X2;
	const int DX20 = X2 - X0;

	const int DY01 = Y0 - Y1;
	const int DY12 = Y1 - Y2;
	const int DY20 = Y2 - Y0;

	// Fixed-point deltas
	const int FDX01 = DX01 << precission;
	const int FDX12 = DX12 << precission;
	const int FDX20 = DX20 << precission;

	const int FDY01 = DY01 << precission;
	const int FDY12 = DY12 << precission;
	const int FDY20 = DY20 << precission;

	// Bounding rectangle
	int minx = (mymin(mymin(X0, X1), X2) + mask) >> precission;
	int maxx = (mymax(mymax(X0, X1), X2) + mask) >> precission;
	int miny = (mymin(mymin(Y0, Y1), Y2) + mask) >> precission;
	int maxy = (mymax(mymax(Y0, Y1), Y2) + mask) >> precission;

	// Half-edge constants
	int C0 = DY01 * X0 - DX01 * Y0;
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY20 * X2 - DX20 * Y2;

	// Correct for fill convention
	if (DY01 < 0 || (DY01 == 0 && DX01 > 0)) C0++;
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if (DY20 < 0 || (DY20 == 0 && DX20 > 0)) C2++;

	int CY0 = C0 + DX01 * (miny << precission) - DY01 * (minx << precission);
	int CY1 = C1 + DX12 * (miny << precission) - DY12 * (minx << precission);
	int CY2 = C2 + DX20 * (miny << precission) - DY20 * (minx << precission);

	auto iC = 1.0f / (CY0 + CY1 + CY2);

	for (int y = miny; y < maxy; y++)
	{
		if (y >= 0 && y < m_szViewport.cy)
		{
			int CX0 = CY0;
			int CX1 = CY1;
			int CX2 = CY2;

			for (int x = minx; x < maxx; x++)
			{
				if (x >= 0 && x < m_szViewport.cx)
				{
					if (CX0 > 0 && CX1 > 0 && CX2 > 0)
					{
						const auto r = (c2.Value.x * CX0 + c0.Value.x * CX1 + c1.Value.x * CX2) * iC;
						const auto g = (c2.Value.y * CX0 + c0.Value.y * CX1 + c1.Value.y * CX2) * iC;
						const auto b = (c2.Value.z * CX0 + c0.Value.z * CX1 + c1.Value.z * CX2) * iC;
						const auto a = (c2.Value.w * CX0 + c0.Value.w * CX1 + c1.Value.w * CX2) * iC;
						const auto u = (u2 * CX0 + u0 * CX1 + u1 * CX2) * iC;
						const auto v = (v2 * CX0 + v0 * CX1 + v1 * CX2) * iC;

						const auto tx = mymax(0, mymin(static_cast<int>(u * (m_nFontTextureWidth)), m_nFontTextureWidth - 1));
						const auto ty = mymax(0, mymin(static_cast<int>(v * (m_nFontTextureHeight)), m_nFontTextureHeight - 1));

						const auto texture_color = m_pImGuiFontTexture[tx + ty * m_nFontTextureWidth] * (1.0f / 255.0f);

						blend_color_c(x, y, r, g, b, texture_color * a);
					}
				}
				else if (x >= m_szViewport.cx)
					break;

				CX0 -= FDY01;
				CX1 -= FDY12;
				CX2 -= FDY20;
			}
		}
		else if (y >= m_szViewport.cy)
			return;

		CY0 += FDX01;
		CY1 += FDX12;
		CY2 += FDX20;
	}
}

void C3DProjector::blend_color(int x, int y, float c, float a)
{
	pixel_t back(*((uint32_t*) &m_pCurrentFrameBuffer[x*4 + y * m_nStride]));
	auto	pixel = color_to_pixel(c);

	auto ia = (int)(a * 255);

	pixel.r = (int)back.r + ((int)pixel.r - (int)back.r) * ia / 255;
	pixel.g = (int)back.g + ((int)pixel.g - (int)back.g) * ia / 255;
	pixel.b = (int)back.b + ((int)pixel.b - (int)back.b) * ia / 255;

	*((uint32_t*) &m_pCurrentFrameBuffer[x*4 + y * m_nStride]) = pixel.integer;
}

void C3DProjector::blend_color_c(int x, int y, float r, float g, float b, float a)
{
	pixel_t back(*((uint32_t*) &m_pCurrentFrameBuffer[x*4 + y * m_nStride]));
	pixel_t	pixel(r * 255, g * 255, b * 255, 255);

	auto ia = (int)(a * 255);

	pixel.r = (int)back.r + ((int)pixel.r - (int)back.r) * ia / 255;
	pixel.g = (int)back.g + ((int)pixel.g - (int)back.g) * ia / 255;
	pixel.b = (int)back.b + ((int)pixel.b - (int)back.b) * ia / 255;

	*((uint32_t*) &m_pCurrentFrameBuffer[x*4 + y * m_nStride]) = pixel.integer;
}
