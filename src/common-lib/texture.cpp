#include "texture.h"

ConstTexture::ConstTexture(const XMCOLOR& color)
{
	m_color = XMLoadColor(&color);
}

XMVECTOR ConstTexture::Evaluate(XMFLOAT2 uv) const
{
	return m_color;
}

CheckerTexture::CheckerTexture(const XMCOLOR& color0, const XMCOLOR& color1, float tiling) :
	m_tilingScale{tiling}
{
	m_checkerColors[0] = XMLoadColor(&color0);
	m_checkerColors[1] = XMLoadColor(&color1);
}

XMVECTOR CheckerTexture::Evaluate(XMFLOAT2 uv) const
{
	const auto u = static_cast<int>(m_tilingScale * uv.x);
	const auto v = static_cast<int>(m_tilingScale * uv.y);

	if (u % 2 == v % 2)
	{
		return m_checkerColors[0];
	}
	else
	{
		return m_checkerColors[1];
	}
}