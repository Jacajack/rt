#pragma once

#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include "ray.hpp"

namespace rt {

/**
	Axis-aligned bounding box
*/
struct aabb : public ray_intersectable
{
	aabb() = default;

	aabb(const glm::vec3 &mi, const glm::vec3 &ma) :
		min(mi),
		max(ma)
	{}

	aabb(const aabb &lhs, const aabb &rhs) :
		min(glm::min(lhs.min, rhs.min)),
		max(glm::max(lhs.max, rhs.max))
	{}

	inline bool check_point_inside(const glm::vec3 &p, float epsilon = 0.f) const;
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;
	inline bool check_ray_intersect(const ray &r) const;
	inline float ray_intersection_distance(const ray &r) const;

	glm::vec3 get_size() const
	{
		return max - min;
	}

	glm::vec3 get_center() const
	{
		return (max + min) / 2.f;
	}

	glm::vec3 min;
	glm::vec3 max;	
};

/**
	Checks whether a point is inside the box. Includes points on the faces
*/
inline bool aabb::check_point_inside(const glm::vec3 &p, float eps) const
{
	return 	   min.x - eps <= p.x && p.x <= max.x + eps
			&& min.y - eps <= p.y && p.y <= max.y + eps
			&& min.z - eps <= p.z && p.z <= max.z + eps;
}

/**
	This is only for compatibilty with other intersectable objects.

	\warning Returned normals are not real
*/
inline bool aabb::ray_intersect(const ray &r, ray_intersection &hit) const
{
	float t = ray_intersection_distance(r);
	if (t != HUGE_VALF)
	{
		hit.distance = t;
		hit.position = r.origin + t * r.direction;
		hit.direction = r.direction;
		hit.normal = -r.direction;
		return true;
	}
	
	return false;
}

/**
	Returns true if ray intersects AABB
*/
inline bool aabb::check_ray_intersect(const ray &r) const
{
	return ray_intersection_distance(r) != HUGE_VALF;
}

/**
	Returns distance to ray-box intersection. Returns HUGE_VALF on miss
*/
inline float aabb::ray_intersection_distance(const ray &r) const
{
	// The intersection point
	float t = HUGE_VALF;

	// Check YZ planes
	if (r.direction.x != 0.f)
	{
		float t1 = (min.x - r.origin.x) / r.direction.x;
		float t2 = (max.x - r.origin.x) / r.direction.x;
		
		// Pick closer positive
		if (t1 > 0 && t2 > 0)
		{
			float u = std::min(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
		else if (t1 * t2 < 0)
		{
			float u = std::max(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
	}

	// Check XZ planes
	if (t == HUGE_VALF && r.direction.y != 0.f)
	{
		float t1 = (min.y - r.origin.y) / r.direction.y;
		float t2 = (max.y - r.origin.y) / r.direction.y;
		
		// Pick closer positive
		if (t1 > 0 && t2 > 0)
		{
			float u = std::min(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
		else if (t1 * t2 < 0)
		{
			float u = std::max(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
	}

	// Check ZY planes
	if (t == HUGE_VALF && r.direction.z != 0.f)
	{
		float t1 = (min.z - r.origin.z) / r.direction.z;
		float t2 = (max.z - r.origin.z) / r.direction.z;
		
		// Pick closer positive
		if (t1 > 0 && t2 > 0)
		{
			float u = std::min(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
		else if (t1 * t2 < 0)
		{
			float u = std::max(t1, t2);
			glm::vec3 v = r.origin + r.direction * u;
			if (check_point_inside(v, 0.001f))
				t = u;
		}
	}

	return t;
}

/**
	Abstract base class for anything that can be bounded with AABB
*/
class aabb_provider
{
public:
	virtual aabb get_aabb() const = 0;
};

}