#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

class Camera
{
public:
	Camera(XMVECTOR origin, XMVECTOR lookAt, float verticalFOV, float aspectRatio, float focalLength, float aperture);
	Ray GetRay(XMFLOAT2 uv, XMFLOAT2 offset) const;
	XMVECTOR GetOrigin() const;

private:
	XMVECTOR m_origin;
	XMVECTOR m_x;
	XMVECTOR m_y;
	XMVECTOR m_originImagePlane;
	float m_aperture;
	float m_focalLength;
};
