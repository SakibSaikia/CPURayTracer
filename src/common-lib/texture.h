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