#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

class Texture
{
public:
	virtual XMVECTOR Evaluate(XMFLOAT2 uv) const = 0;
};

class ConstTexture : public Texture
{
public:
	ConstTexture(const XMCOLOR& color);
	XMVECTOR Evaluate(XMFLOAT2 uv) const override;

private:
	XMVECTOR m_color;
};

class CheckerTexture : public Texture
{
public:
	CheckerTexture(const XMCOLOR& color0, const XMCOLOR& color1, float tiling);
	XMVECTOR Evaluate(XMFLOAT2 uv) const override;

private:
	std::array<XMVECTOR, 2> m_checkerColors;
	float m_tilingScale;
};