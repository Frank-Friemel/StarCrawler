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

glm::dvec3  C3DWord::GetBoundingBox() const
{
	ATLASSERT(!m_listChars.empty());
	return (*m_listChars.rbegin())->m_vecVertices[1] - (*m_listChars.begin())->m_vecVertices[0];
}

void C3DWord::Draw(C3DProjector* pProjector)
{
	for each (auto& c in m_listChars)
		c->Draw(pProjector);
}

bool C3DWord::Create(PCWSTR strWord, const LOGFONT* pLogFont, bool bHinted /*= false*/)
{
	while (*strWord)
	{
		auto c = std::make_shared<C3DGlyph>();

		if (!c->Create(*strWord, pLogFont, bHinted))
			return false;

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
