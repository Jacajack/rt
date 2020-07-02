#pragma once

#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include "ray.hpp"

namespace rt {

/**
	Axis-aligned bounding box
*/
class aabb : public ray_intersectable
{
public:
	aabb() = default;

	aabb(const glm::vec3 &mi, const glm::vec3 &ma) :
		min(mi),
		max(ma),
		size(max - min),
		half_size(size / 2.f),
		center((max + min) / 2.f)
	{}

	aabb(const aabb &lhs, const aabb &rhs) :
		min(glm::min(lhs.min, rhs.min)),
		max(glm::max(lhs.max, rhs.max)),
		size(max - min),
		half_size(size / 2.f),
		center((max + min) / 2.f)
	{}

	inline bool check_aabb_overlap(const aabb &rhs) const;
	inline bool check_point_inside(const glm::vec3 &p, float epsilon = 0.f) const;
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;
	inline bool check_ray_intersect(const ray &r) const;
	inline float ray_intersection_distance(const ray &r) const;

	const glm::vec3 &get_min() const
	{
		return min;
	}

	const glm::vec3 &get_max() const
	{
		return max;
	}

	const glm::vec3 &get_size() const
	{
		return size;
	}

	const glm::vec3 &get_half_size() const
	{
		return half_size;
	}

	const glm::vec3 &get_center() const
	{
		return center;
	}

private:
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 size;
	glm::vec3 half_size;
	glm::vec3 center;
};

/**
	Returns true if two AABBs overlap
*/
inline bool aabb::check_aabb_overlap(const aabb &rhs) const
{
	return	   min.x < rhs.max.x && max.x > rhs.min.x
			&& min.y < rhs.max.y && max.y > rhs.min.y
			&& min.z < rhs.max.z && max.z > rhs.min.z;
}


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
	Returns closest distance to ray-box intersection. Returns HUGE_VALF on miss
*/
inline float aabb::ray_intersection_distance(const ray &r) const
{
	// The intersection point
	float t = HUGE_VALF;

	// Transfrom into AABB coordinate system
	glm::vec3 d{r.direction};
	glm::vec3 o{r.origin - get_center()};
	const glm::vec3 &half{get_half_size()};

	// Mul determines whether the ray is going to be intersected with positive
	// or negative side of the box.
	// This thing here is actually significantly faster than 'nice' glm equivalent
	// glm::vec3 mul{(glm::vec3{glm::lessThan(glm::abs(o), half)} * 2.f - 1.f) * glm::sign(d)};
	glm::vec3 mul{
		(std::abs(o.x) < half.x) == (d.x > 0) ? 1.f : -1.f,
		(std::abs(o.y) < half.y) == (d.y > 0) ? 1.f : -1.f,
		(std::abs(o.z) < half.z) == (d.z > 0) ? 1.f : -1.f,
	};

	// Calculate distances to plane intersections
	// Undefined behavior for rays parallel to axes? Maybe.
	glm::vec3 T{(half * mul - o) / d};
	
	// For each side, calculate intersection and check if it lies within the box 
	glm::vec3 p;
	
	if (T.x > 0)
	{
		p.y = o.y + d.y * T.x;
		p.z = o.z + d.z * T.x;
		if (std::abs(p.y) < half.y && std::abs(p.z) < half.z) 
			t = T.x;
	}

	if (T.y > 0)
	{
		p.x = o.x + d.x * T.y;
		p.z = o.z + d.z * T.y;
		if (std::abs(p.x) < half.x && std::abs(p.z) < half.z) 
			t = std::min(t, T.y);
	}

	if (T.z > 0)
	{
		p.x = o.x + d.x * T.z;
		p.y = o.y + d.y * T.z;
		if (std::abs(p.y) < half.y && std::abs(p.x) < half.x) 
			t = std::min(t, T.z);
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