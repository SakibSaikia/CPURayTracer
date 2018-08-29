#include "camera.h"

Camera::Camera(
	const XMVECTOR origin,
	const XMVECTOR lookAt,
	const float verticalFOV,
	const float aspectRatio,
	const float focalLength,
	const float aperture) :
	m_origin{ origin },
	m_aperture{ aperture },
	m_focalLength{ focalLength }
{
	const float theta = verticalFOV * XM_PI / 180.f;
	const float halfHeight = std::tan(theta / 2.f);
	const float halfWidth = aspectRatio * halfHeight;

	const XMVECTORF32 up{ 0.f, 1.f, 0.f };
	const XMVECTOR w = XMVector3Normalize(origin - lookAt);
	const XMVECTOR u = XMVector3Normalize(XMVector3Cross(up, w));
	const XMVECTOR v = XMVector3Cross(w, u);

	m_originImagePlane = origin - w;
	m_x = halfWidth * u;
	m_y = halfHeight * v;
}

Ray Camera::GetRay(XMFLOAT2 uv, XMFLOAT2 offset) const
{
	// Convert to [-1,1] range
	XMFLOAT2 ndc;
	ndc.x = 2.f * uv.x - 1.f;
	ndc.y = -2.f * uv.y + 1.f;

	// Use primary ray to determine focal point
	const XMVECTOR p = m_originImagePlane + ndc.x * m_x + ndc.y * m_y;
	const XMVECTOR focalPoint = m_origin + m_focalLength * XMVector3Normalize(p - m_origin);

	// Secondary ray used for tracing
	XMFLOAT2 rd;
	rd.x = 0.5f * m_aperture * offset.x;
	rd.y = 0.5f * m_aperture * offset.y;
	const XMVECTOR origin = m_origin + rd.x * m_x + rd.y * m_y;

	return Ray{ origin, XMVector3Normalize(focalPoint - origin) };
}