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
		rgba = i;
	}
	_pixel_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	uint32_t rgba;

	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};
} pixel_t;


C3DProjector::C3DProjector(double zCam /*= 10*/, double zViewer /*= 1200*/)
{
	m_lfWidth	= 0;
	m_lfHeight	= 0;
	m_centerH	= 0;
	m_centerV	= 0;
	m_nStride	= 0;
	m_bClip		= false;

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

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_Tab]			= VK_TAB;                       
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
	io.Fonts->GetTexDataAsAlpha8(&m_pImGuiTexture, &m_nTextureWidth, &m_nTextureHeight);

	// Store our identifier
	io.Fonts->TexID					= m_pImGuiTexture;

	// Init timers
	QueryPerformanceCounter(&m_ImGuiPrevTime);
	
	LARGE_INTEGER liTicksPerSecond;
	QueryPerformanceFrequency(&liTicksPerSecond);
	m_TicksPerSecond = static_cast<float>(liTicksPerSecond.QuadPart);
}

void C3DProjector::ShutdownImGui()
{
	ImGui::Shutdown();
}

void C3DProjector::RenderImGui()
{
	ImGui::Render();
}

void C3DProjector::NewFrameImGui()
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	io.DisplaySize				= ImVec2((float)m_lfWidth, (float)m_lfHeight);
	io.DisplayFramebufferScale	= ImVec2(1, 1);

	// Setup time step
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	
	io.DeltaTime	= (float)(current_time.QuadPart - m_ImGuiPrevTime.QuadPart) / m_TicksPerSecond;
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
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

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

					SetScissors(static_cast<int>(pcmd->ClipRect.x), static_cast<int>(round(pcmd->ClipRect.y)), static_cast<int>(pcmd->ClipRect.z), static_cast<int>(round(pcmd->ClipRect.w)));

					if (!pcmd->TextureId || !TriangleDraw(v0, v2, v1))
					{
						PIXEL2D pxl[4];
						BYTE	cmd[4];

						pxl[0].x	= v0.pos.x;
						pxl[0].y	= v0.pos.y;
						cmd[0]		= PT_MOVETO;

						pxl[1].x	= v2.pos.x;
						pxl[1].y	= v2.pos.y;
						cmd[1]		= PT_LINETO;

						pxl[2].x	= v1.pos.x;
						pxl[2].y	= v1.pos.y;
						cmd[2]		= PT_LINETO;

						pxl[3].x	= v0.pos.x+1;
						pxl[3].y	= v0.pos.y+1;
						cmd[3]		= PT_LINETO;

						const auto c0 = ImColor(v0.col);

						PolyDraw(pxl, cmd, 4, c0.Value.x, c0.Value.y, c0.Value.z, c0.Value.w);
					}
					ClearScissors();
				}
			}
			idx_offset += pcmd->ElemCount;
		}
	}
}

// http://forum.devmaster.net/t/advanced-rasterization/6145
bool C3DProjector::TriangleDraw(const ImDrawVert& vertex0, const ImDrawVert& vertex1, const ImDrawVert& vertex2)
{
	bool bDrawed = false;

	const auto vx0 = vertex0.pos.x;
	const auto vy0 = vertex0.pos.y;

	const auto vx1 = vertex1.pos.x;
	const auto vy1 = vertex1.pos.y;

	const auto vx2 = vertex2.pos.x;
	const auto vy2 = vertex2.pos.y;

	const auto vu0 = vertex0.uv.x;
	const auto vv0 = vertex0.uv.y;

	const auto vu1 = vertex1.uv.x;
	const auto vv1 = vertex1.uv.y;

	const auto vu2 = vertex2.uv.x;
	const auto vv2 = vertex2.uv.y;

	const auto c0 = ImColor(vertex0.col);
	const auto c1 = ImColor(vertex1.col);
	const auto c2 = ImColor(vertex2.col);

	// 24.8 fixed-point
	const int precission = 4;
	const int mask       = (1 << precission) - 1;

	// Fixed-point coordinates
	const int Y1 = (int)(vy0 * static_cast<float>(1 << precission));
	const int Y2 = (int)(vy1 * static_cast<float>(1 << precission));
	const int Y3 = (int)(vy2 * static_cast<float>(1 << precission));

	const int X1 = (int)(vx0 * static_cast<float>(1 << precission));
	const int X2 = (int)(vx1 * static_cast<float>(1 << precission));
	const int X3 = (int)(vx2 * static_cast<float>(1 << precission));

	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;

	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << precission;
	const int FDX23 = DX23 << precission;
	const int FDX31 = DX31 << precission;

	const int FDY12 = DY12 << precission;
	const int FDY23 = DY23 << precission;
	const int FDY31 = DY31 << precission;

	// Bounding rectangle
	int minx = (mymin(mymin(X1, X2), X3) + mask) >> precission;
	int maxx = (mymax(mymax(X1, X2), X3) + mask) >> precission;
	int miny = (mymin(mymin(Y1, Y2), Y3) + mask) >> precission;
	int maxy = (mymax(mymax(Y1, Y2), Y3) + mask) >> precission;

	// Block size, standard 8x8 (must be power of two)
	const int q = 8;

	// Start in corner of 8x8 block
	minx &= ~(q - 1);
	miny &= ~(q - 1);

	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) 
		C1++;
	if (DY23 < 0 || (DY23 == 0 && DX23 > 0))
		C2++;
	if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) 
		C3++;

	const int red		= static_cast<int>((c2.Value.x + c0.Value.x + c1.Value.x) * 255 / 3);
	const int green		= static_cast<int>((c2.Value.y + c0.Value.y + c1.Value.y) * 255 / 3);
	const int blue		= static_cast<int>((c2.Value.z + c0.Value.z + c1.Value.z) * 255 / 3);
	const auto vu		= (vu2 + vu0 + vu1) / 3;
	const auto vv		= (vv2 + vv0 + vv1) / 3;
	const auto vtx		= mymax(0, mymin(static_cast<int>(vu * m_nTextureWidth), m_nTextureWidth - 1));
	const auto vty		= mymax(0, mymin(static_cast<int>(vv * m_nTextureHeight), m_nTextureHeight - 1));
	const int alpha		= static_cast<int>((c2.Value.w + c0.Value.w + c1.Value.w) / 3.0f * m_pImGuiTexture[vtx + vty * m_nTextureWidth]);

	// Loop through blocks
	for (int y = miny; y < maxy; y += q)
	{
		if (y >= m_szViewport.cy)
			break;

		for (int x = minx; x < maxx; x += q)
		{
			if (x >= m_szViewport.cx)
				break;

			// Corners of block
			int x0 = x << 4;
			int x1 = (x + q - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + q - 1) << 4;

			// Evaluate half-space functions
			bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			
			int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

			bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			
			int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

			bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			
			int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

			// Skip block when outside an edge
			if (a == 0x0 || b == 0x0 || c == 0x0)
				continue;

			// Accept whole block when totally covered
			if (a == 0xF && b == 0xF && c == 0xF)
			{
				for (int iy = y; iy < y + q; ++iy)
				{
					if (iy >= 0 && iy < m_szViewport.cy)
					{
						for (int ix = x; ix < x + q; ++ix)
						{
							if (ix >= 0 && ix < m_szViewport.cx)
							{
								BlendColor(ix, iy, red, green, blue, alpha);
								bDrawed = true;
							}
							else if (ix >= m_szViewport.cx)
								break;
						}
					}
					else if (iy >= m_szViewport.cy)
						break;
				}
			}
			else // Partially covered block
			{
				int CY1 = C1 + DX12 * y0 - DY12 * x0;
				int CY2 = C2 + DX23 * y0 - DY23 * x0;
				int CY3 = C3 + DX31 * y0 - DY31 * x0;

				auto iC = 1.0f / (CY1 + CY2 + CY3);

				for (int iy = y; iy < y + q; ++iy)
				{
					if (iy >= 0 && iy < m_szViewport.cy)
					{
						int CX1 = CY1;
						int CX2 = CY2;
						int CX3 = CY3;

						for (int ix = x; ix < x + q; ++ix)
						{
							if (ix >= 0 && ix < m_szViewport.cx)
							{
								if (CX1 > 0 && CX2 > 0 && CX3 > 0)
								{
									const auto r = (c2.Value.x * CX1 + c0.Value.x * CX2 + c1.Value.x * CX3) * iC;
									const auto g = (c2.Value.y * CX1 + c0.Value.y * CX2 + c1.Value.y * CX3) * iC;
									const auto b = (c2.Value.z * CX1 + c0.Value.z * CX2 + c1.Value.z * CX3) * iC;
									const auto u = (vu2 * CX1 + vu0 * CX2 + vu1 * CX3) * iC;
									const auto v = (vv2 * CX1 + vv0 * CX2 + vv1 * CX3) * iC;

									const auto tx = mymax(0, mymin(static_cast<int>(u * m_nTextureWidth), m_nTextureWidth - 1));
									const auto ty = mymax(0, mymin(static_cast<int>(v * m_nTextureHeight), m_nTextureHeight - 1));

									const auto a = (c2.Value.w * CX1 + c0.Value.w * CX2 + c1.Value.w * CX3) * iC * m_pImGuiTexture[tx + ty * m_nTextureWidth];

									BlendColor(ix, iy, static_cast<int>(r*255), static_cast<int>(g*255), static_cast<int>(b*255), static_cast<int>(a));
									bDrawed = true;
								}
							}
							else if (ix >= m_szViewport.cx)
								break;

							CX1 -= FDY12;
							CX2 -= FDY23;
							CX3 -= FDY31;
						}
					}
					else if (iy >= m_szViewport.cy)
						break;
					
					CY1 += FDX12;
					CY2 += FDX23;
					CY3 += FDX31;
				}
			}
		}
	}
	return bDrawed;
}


void C3DProjector::BlendColor(int x, int y, int r, int g, int b, int a)
{
	if (!m_bClip || (x >= m_rectClip.left && x <= m_rectClip.right && y >= m_rectClip.top && y <= m_rectClip.bottom))
	{
		auto back = (pixel_t*) &m_pCurrentFrameBuffer[x*sizeof(pixel_t) + y * m_nStride];

		back->r += ((r - back->r) * a / 255);
		back->g += ((g - back->g) * a / 255);
		back->b += ((b - back->b) * a / 255);
	}
}
