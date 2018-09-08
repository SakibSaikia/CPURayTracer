#include "texture.h"

ConstTexture::ConstTexture(const XMCOLOR& color)
{
	m_color = XMLoadColor(&color);
}

XMVECTOR ConstTexture::Evaluate(XMFLOAT2 uv) const
{
	return m_color;
}