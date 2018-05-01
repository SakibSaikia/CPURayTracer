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


bool Sphere::Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload)
{
	XMVECTOR oc = ray.origin - center;

	XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	XMVECTOR b = XMVector3Dot(oc, ray.direction);
	XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	XMVECTOR discriminant = b*b - a*c;

	if (XMVectorGetX(discriminant) > 0.f)
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / a;
		XMVECTOR valid = XMVectorAndInt(XMVectorGreater(t, tmin), XMVectorLess(t, tmax));

		if(XMVectorGetX(valid) != 0 )
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;
		valid = XMVectorAndInt(XMVectorGreater(t, tmin), XMVectorLess(t, tmax));

		if (XMVectorGetX(valid) != 0)
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);

			return true;
		}
	}

	return false;
}