#pragma once

using namespace DirectX;

__declspec(align(16))
struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	Ray(const XMVECTOR& o, const XMVECTOR& d);
	XMVECTOR Evaluate(float t);
};

