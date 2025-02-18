#include "stdafx.h"
#include "3DGlyph.h"

#ifndef GGO_UNHINTED
#define GGO_UNHINTED 0x0100
#endif

static inline double fx_to_dbl(const FIXED& p)
{
	return double(p.value) + double(p.fract) * (1.0 / 65536.0);
}

C3DGlyph::C3DGlyph()
{
}

C3DGlyph::~C3DGlyph()
{
}

glm::dvec3  C3DGlyph::GetBoundingBox() const
{
	ATLASSERT(!m_vecVertices.empty());
	return m_vecVertices[1] - m_vecVertices[0];
}

bool C3DGlyph::Create(WCHAR c, const LOGFONT* pLogFont, bool bHinted /*= false*/)
{
	Clear();

	CClientDC hdc(NULL);

	ATLASSERT(pLogFont);

	// create font and select it
	CFont newFont;

	if (!newFont.CreateFontIndirect(pLogFont))
	{
		return false;
	}
	const double lfScale = 25.0;

	bool	bResult = false;
	auto	pOldFont = hdc.SelectFont(newFont);

	GLYPHMETRICS gm = { 0 };

	MAT2 mat2 = { 0 };

	mat2.eM11.value = 1;
	mat2.eM22.value = 1;

	ULONG buf_size = 16384 - 32;
	CTempBuffer<uint8_t>	gbuf(buf_size);

	int total_size = ::GetGlyphOutlineW(hdc, c, GGO_NATIVE | (bHinted ? 0 : GGO_UNHINTED), &gm, buf_size, gbuf, &mat2);

	if (total_size > 0)
	{
		bResult = true;

		// store bounding box (lower-left, upper-right)
		m_vecVertices.push_back(glm::dvec4(0, 0, 0, 1));
		m_vecVertices.push_back(glm::dvec4((double)((gm.gmBlackBoxX > (UINT)gm.gmCellIncX) ? gm.gmBlackBoxX : (UINT)gm.gmCellIncX) / lfScale, ((double)gm.gmBlackBoxY) / lfScale * (-1.0), 0, 1));

		const uint8_t* cur_glyph = gbuf;
		const uint8_t* end_glyph = gbuf + total_size;

		while (cur_glyph < end_glyph)
		{
			const TTPOLYGONHEADER* th = (const TTPOLYGONHEADER*)cur_glyph;

			const uint8_t* end_poly = cur_glyph + th->cb;
			const uint8_t* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

			m_vecVertices.push_back(glm::dvec4(fx_to_dbl(th->pfxStart.x) / lfScale, -fx_to_dbl(th->pfxStart.y) / lfScale, 0, 1));
			m_vecFaces.push_back(face_t(m_vecVertices.size()-1, PT_MOVETO));

			while (cur_poly < end_poly)
			{
				const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

				if (pc->wType == TT_PRIM_LINE)
				{
					for (int i = 0; i < pc->cpfx; i++)
					{
						m_vecVertices.push_back(glm::dvec4(fx_to_dbl(pc->apfx[i].x) / lfScale, -fx_to_dbl(pc->apfx[i].y) / lfScale, 0, 1));
						m_vecFaces.push_back(face_t(m_vecVertices.size() - 1, PT_LINETO));
					}
				}
				else if (pc->wType == TT_PRIM_QSPLINE)
				{
					for (int u = 0; u < pc->cpfx - 1; u++)
					{
						// B is always the current point
						POINTFX pnt_b = pc->apfx[u];    
						POINTFX pnt_c = pc->apfx[u + 1];

						// if not on last spline, compute C
						if (u < pc->cpfx - 2)          
						{
							// midpoint (x,y)
							*(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
							*(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
						}

						m_vecVertices.push_back(glm::dvec4(fx_to_dbl(pnt_b.x) / lfScale, -fx_to_dbl(pnt_b.y) / lfScale, 0, 1));
						m_vecFaces.push_back(face_t(m_vecVertices.size() - 1, PT_BEZIERTO));

						m_vecVertices.push_back(glm::dvec4(fx_to_dbl(pnt_c.x) / lfScale, -fx_to_dbl(pnt_c.y) / lfScale, 0, 1));
						m_vecFaces.push_back(face_t(m_vecVertices.size() - 1, PT_BEZIERTO));
					}
				}
				cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
			}
			cur_glyph += th->cb;

			m_vecVertices.push_back(glm::dvec4(0, 0, 0, 1));
			m_vecFaces.push_back(face_t(m_vecVertices.size() - 1, PT_CCW_EX));
		}
		m_vecVertices.push_back(glm::dvec4(0, 0, 0, 1));
		m_vecFaces.push_back(face_t(m_vecVertices.size() - 1, PT_STOP));
	}
	// put back the old font
	hdc.SelectFont(pOldFont);

	return bResult;
}
