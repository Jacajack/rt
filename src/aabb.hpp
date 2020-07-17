#pragma once

#include <set>
#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include "ray.hpp"
#include <stdexcept>

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

	const float get_surface_area() const
	{
		return 2.f * (size.x * size.y + size.x * size.z + size.z * size.y);
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
*/
inline bool aabb::ray_intersect(const ray &r, ray_intersection &hit) const
{
	float t = ray_intersection_distance(r);
	if (t != rt::ray_miss)
	{
		hit.distance = t;
		hit.u = hit.v = 0.f;
		return true;
	}
	
	return false;
}

/**
	Returns true if ray intersects AABB
*/
inline bool aabb::check_ray_intersect(const ray &r) const
{
	return ray_intersection_distance(r) != rt::ray_miss;
}

/**
	Returns closest distance to ray-box intersection. Returns HUGE_VALF on miss

	Based on: https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
*/
inline float aabb::ray_intersection_distance(const ray &r) const
{
	glm::vec3 a{(min - r.origin) / r.direction};
	glm::vec3 b{(max - r.origin) / r.direction};

	float tmin = std::max(std::max(std::min(a.x, b.x), std::min(a.y, b.y)), std::min(a.z, b.z));
	float tmax = std::min(std::min(std::max(a.x, b.x), std::max(a.y, b.y)), std::max(a.z, b.z));

	if (tmax < 0 || tmin > tmax) return rt::ray_miss;
	else return tmin;
}

/**
	Abstract base class for anything that can be bounded with AABB
*/
class aabb_provider
{
public:
	virtual aabb get_aabb() const = 0;
};

/**
	Meant for computing AABB of smaller boundig boxes.

	\warning Based on direct float comparisons. Provided values
	should be always the same and not calculated each time.
*/
class aabb_collection : public aabb_provider
{
public:
	aabb_collection() = default;

	template <typename T>
	aabb_collection(T begin, T end)
	{
		// static_assert(std::is_base_of_v<aabb_provider, *T>);

		for (auto it = begin; it != end; ++it)
		{
			auto min = it->get_aabb().get_min();
			auto max = it->get_aabb().get_max();
			
			m_x.insert(min.x);
			m_y.insert(min.y);
			m_z.insert(min.z);

			m_x.insert(max.x);
			m_y.insert(max.y);
			m_z.insert(max.z);
		}
	}

	void push(const aabb &box)
	{
		m_x.insert(box.get_min().x);
		m_y.insert(box.get_min().y);
		m_z.insert(box.get_min().z);

		m_x.insert(box.get_max().x);
		m_y.insert(box.get_max().y);
		m_z.insert(box.get_max().z);
	}

	/**
		\todo Fix this
	*/
	void pop(const aabb &box)
	{
		if (m_x.empty() || m_y.empty() || m_z.empty())
			throw std::out_of_range("pop() called on (at least partially) empty aabb_collection");

		auto min_x_it = m_x.find(box.get_min().x);
		auto min_y_it = m_y.find(box.get_min().y);
		auto min_z_it = m_z.find(box.get_min().z);

		if (min_x_it == m_x.end() || min_y_it == m_y.end() || min_z_it == m_z.end())
			throw std::out_of_range("popped invalid value from aabb_collection");

		m_x.erase(min_x_it);
		m_y.erase(min_y_it);
		m_z.erase(min_z_it);

		auto max_x_it = m_x.find(box.get_max().x);
		auto max_y_it = m_y.find(box.get_max().y);
		auto max_z_it = m_z.find(box.get_max().z);

		if (max_x_it == m_x.end() || max_y_it == m_y.end() || max_z_it == m_z.end())
			throw std::out_of_range("popped invalid value from aabb_collection");

		m_x.erase(max_x_it);
		m_y.erase(max_y_it);
		m_z.erase(max_z_it);
	}

	aabb get_aabb() const override
	{
		if (m_x.empty() || m_y.empty() || m_z.empty())
			throw std::out_of_range("get_aabb() called on (at least partially) empty aabb_collection");

		glm::vec3 min{
			*m_x.begin(),
			*m_y.begin(),
			*m_z.begin()
		};

		glm::vec3 max{
			*m_x.rbegin(),
			*m_y.rbegin(),
			*m_z.rbegin()
		};

		return aabb{min, max};
	}

private:
	std::multiset<float> m_x, m_y, m_z;
};

}