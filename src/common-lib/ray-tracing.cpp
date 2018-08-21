#include "ray-tracing.h"
#include "quasi-random.h"
#include "material.h"

Ray::Ray(const XMVECTOR& o, const XMVECTOR& d) noexcept :
	origin{ o }, direction{ d } 
{
}

XMVECTOR Ray::Evaluate(float t)
{
	// origin + t * direction
	return XMVectorMultiplyAdd(direction, XMVectorReplicate(t), origin);
}

Sphere::Sphere(const XMVECTOR& c, const float r, std::unique_ptr<Material>&& mat) noexcept :
	center{ c }, radius{ r }, material{ std::move(mat) }
{
}


bool Sphere::Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload) const
{
	const XMVECTOR oc = ray.origin - center;

	const XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	const XMVECTOR b = XMVector3Dot(oc, ray.direction);
	const XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	const XMVECTOR discriminant = b*b - a*c;

	if (XMVector3Greater(discriminant, XMVectorZero()))
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.material = material.get();

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.material = material.get();

			return true;
		}
	}

	return false;
}

AABB Sphere::GetAABB() const
{
	XMFLOAT3 origin;
	XMStoreFloat3(&origin, center);

	const XMFLOAT3 extents = { radius, radius, radius };

	return AABB{ origin, extents };
}

AABB::AABB(const XMFLOAT3& center, const XMFLOAT3& extents) : 
	m_aabb{center, extents}
{
}

bool AABB::Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax) const
{
	float t;
	bool anyIntersection = m_aabb.Intersects(ray.origin, ray.direction, t);

	XMVECTOR tVec = XMVectorReplicate(t);
	return anyIntersection && XMVector3Greater(tVec, tmin) && XMVector3Less(tVec, tmax);
}

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

	m_lowerLeft = origin - halfWidth * u - halfHeight * v - w;
	m_x = 2 * halfWidth * u;
	m_y = 2 * halfHeight * v;
}

Ray Camera::GetRay(XMFLOAT2 uv, XMFLOAT2 offset) const
{
	// Use primary ray to determine focal point
	const XMVECTOR p = m_lowerLeft + uv.x * m_x + uv.y * m_y;
	const XMVECTOR focalPoint = m_origin + m_focalLength * XMVector3Normalize(p - m_origin);

	// Secondary ray used for tracing
	XMFLOAT2 rd;
	rd.x = 0.5f * m_aperture * offset.x;
	rd.y = 0.5f * m_aperture * offset.y;
	const XMVECTOR origin = m_origin + rd.x * m_x + rd.y * m_y;

	return Ray{ origin, XMVector3Normalize(focalPoint - origin) };
}