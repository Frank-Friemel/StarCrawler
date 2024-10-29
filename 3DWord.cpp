#include "stdafx.h"
#include "3DWord.h"


C3DWord::C3DWord()
{
}

C3DWord::~C3DWord()
{
}

void C3DWord::Clear()
{
	__super::Clear();
	m_listChars.clear();
}

glm::dvec3 C3DWord::GetBoundingBox() const
{
	ATLASSERT(!m_listChars.empty());
	return (*m_listChars.rbegin())->m_vecVertices[1] - (*m_listChars.begin())->m_vecVertices[0];
}

void C3DWord::Draw(I3DProjector* pProjector, uint8_t* frameBuffer) const
{
	for (const auto& c : m_listChars)
	{
		c->Draw(pProjector, frameBuffer);
	}
}

bool C3DWord::Create(PCWSTR strWord, const LOGFONT* pLogFont, bool bHinted /*= false*/)
{
	while (*strWord)
	{
		auto c = std::make_shared<C3DGlyph>();

		if (!c->Create(*strWord, pLogFont, bHinted))
		{
			return false;
		}
		if (!m_listChars.empty())
		{
			glm::dvec3 bounding = GetBoundingBox();
			c->MoveX(bounding.x);
		}
		m_listChars.push_back(c);

		strWord++;
	}
	return true;
}

void C3DWord::RotateX(double lfDegree)
{
	__super::RotateX(lfDegree);

	for (auto& c : m_listChars)
	{
		c->RotateX(lfDegree);
	}
}

void C3DWord::Move(const glm::dmat4& matTrans)
{
	__super::Move(matTrans);

	for (auto& c : m_listChars)
	{
		c->Move(matTrans);
	}
}

void C3DWord::Move(const glm::dvec3& vecTrans)
{
	__super::Move(vecTrans);

	for (auto& c : m_listChars)
	{
		c->Move(vecTrans);
	}
}

void C3DWord::MoveX(double lfTrans)
{
	__super::MoveX(lfTrans);

	for (auto& c : m_listChars)
	{
		c->MoveX(lfTrans);
	}
}

void C3DWord::MoveZ(double lfTrans)
{
	__super::MoveZ(lfTrans);

	for (auto& c : m_listChars)
	{
		c->MoveZ(lfTrans);
	}
}

void C3DWord::Scale(double lfFactor)
{
	__super::Scale(lfFactor);

	for (auto& c : m_listChars)
	{
		c->Scale(lfFactor);
	}
}

void C3DWord::SetColor(double r, double g, double b)
{
	__super::SetColor(r, g, b);

	for (auto& c : m_listChars)
	{
		c->SetColor(r, g, b);
	}
}

void C3DWord::SetAlpha(double value)
{
	if (value >= 0.0 && value <= 1.0)
	{
		__super::SetAlpha(value);

		for (auto& c : m_listChars)
		{
			c->SetAlpha(value);
		}
	}
}