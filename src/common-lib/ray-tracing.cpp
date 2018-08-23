#include "ray-tracing.h"
#include "quasi-random.h"
#include "material.h"

XMFLOAT3 operator-(XMFLOAT3 a, XMFLOAT3 b)
{
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

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


bool Sphere::Intersect(const Ray& ray, Payload& payload) const
{
	const XMVECTOR oc = ray.origin - center;

	const XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	const XMVECTOR b = XMVector3Dot(oc, ray.direction);
	const XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	const XMVECTOR discriminant = b*b - a*c;

	static const XMVECTORF32 bias{ 0.001, 0.001, 0.001 };

	if (XMVector3Greater(discriminant, XMVectorZero()))
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, bias))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.material = material.get();

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, bias))
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
	m_box{center, extents}
{
}

bool AABB::Intersect(const Ray& ray) const
{
	float t;
	return m_box.Intersects(ray.origin, ray.direction, t);
}

BvhNode::BvhNode(BvhNode::Iter begin, BvhNode::Iter end)
{
	size_t n = std::distance(begin, end);

	if (n == 1)
	{
		m_left = std::move(*begin);
		m_right = nullptr;
	}
	else if (n == 2)
	{
		m_left = std::move(*begin);
		m_right = std::move(*(begin + 1));
	}
	else
	{
		switch (auto axis = std::rand() % 3; axis)
		{
		case 0:
			std::sort(begin, end,
				[](const std::unique_ptr<Hitable>& a, const std::unique_ptr<Hitable>& b)
			{
				XMFLOAT3 min_a = a->GetAABB().m_box.Center - a->GetAABB().m_box.Extents;
				XMFLOAT3 min_b = b->GetAABB().m_box.Center - b->GetAABB().m_box.Extents;
				return min_a.x < min_b.x;
			});
			break;
		case 1:
			std::sort(begin, end,
				[](const std::unique_ptr<Hitable>& a, const std::unique_ptr<Hitable>& b)
			{
				XMFLOAT3 min_a = a->GetAABB().m_box.Center - a->GetAABB().m_box.Extents;
				XMFLOAT3 min_b = b->GetAABB().m_box.Center - b->GetAABB().m_box.Extents;
				return min_a.y < min_b.y;
			});
			break;
		case 2:
			std::sort(begin, end,
				[](const std::unique_ptr<Hitable>& a, const std::unique_ptr<Hitable>& b)
			{
				XMFLOAT3 min_a = a->GetAABB().m_box.Center - a->GetAABB().m_box.Extents;
				XMFLOAT3 min_b = b->GetAABB().m_box.Center - b->GetAABB().m_box.Extents;
				return min_a.z < min_b.z;
			});
			break;
		}

		m_left = std::make_unique<BvhNode>(begin, begin + n / 2);
		m_right = std::make_unique<BvhNode>(begin + n / 2, end);
	}

	// Merged AABB
	if (m_right != nullptr)
	{
		DirectX::BoundingBox::CreateMerged(m_aabb.m_box, m_left->GetAABB().m_box, m_right->GetAABB().m_box);
	}
	else
	{
		m_aabb = m_left->GetAABB();
	}
}

AABB BvhNode::GetAABB() const
{
	return m_aabb;
}

bool BvhNode::Intersect(const Ray& ray, Payload& payload) const
{
	if (m_aabb.Intersect(ray))
	{
		Payload leftPayload, rightPayload;
		bool leftHit = m_left->Intersect(ray, leftPayload);
		bool rightHit = (m_right != nullptr ? m_right->Intersect(ray, rightPayload) : false);

		if (leftHit && rightHit)
		{
			if (XMVector3Less(leftPayload.t, rightPayload.t))
			{
				payload = leftPayload;
			}
			else
			{
				payload = rightPayload;
			}

			return true;
		}
		else if (leftHit)
		{
			payload = leftPayload;
			return true;
		}
		else if (rightHit)
		{
			payload = rightPayload;
			return true;
		}
		else
		{
			return false;
		}
	}
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