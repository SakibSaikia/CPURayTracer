#include "stdafx.h"

Ray::Ray(const XMVECTOR& o, const XMVECTOR& d) :
	origin{ o }, direction{ d } 
{
}

XMVECTOR Ray::Evaluate(float t)
{
	// origin + t * direction
	return XMVectorMultiplyAdd(direction, XMVectorReplicate(t), origin);
}

Sphere::Sphere(const XMVECTOR& c, const float r) :
	center{ c }, radius{ r }
{
}


std::optional<XMVECTOR> Sphere::Intersect(const Ray& ray)
{
	XMVECTOR oc = ray.origin - center;

	XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	XMVECTOR b = XMVectorReplicate(2.f) * XMVector3Dot(oc, ray.direction);
	XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	XMVECTOR discriminant = b * b - XMVectorReplicate(4.f) * a * c;

	if (XMVectorGetX(discriminant) >= 0.f)
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / (2.f * a);
		return XMVectorMultiplyAdd(t, ray.direction, ray.origin);
	}
	else
	{
		return std::nullopt;
	}
}