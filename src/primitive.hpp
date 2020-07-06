#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"
#include "aabb.hpp"

/**
	\file primitive.hpp

	Primitives used in the path tracing process - spheres, planes and triangles.

	Each primitive class must satisfy following conditions:
		- must provide ray_intersect() member function to check intersections
		- must provide get_aabb() member function returning primitive's bounding box
		- must provide get_ray_hit() member function used for converting ray_intersection to ray_hit 
		- must contain pointer to abstract_material class
	
	Some of these requirements are enforced by the primitive base class.
	I'm not going to use virtual functions here, because they hurt performance (a couple of ms for each sample).

	get_ray_hit() provides full hit information based on previously acquired ray_intersection.
	ray_intersection passed as an argument must come from the very same primitive.
*/

namespace rt {

/**
	Non-abstract base class for all primitives (perf reasons).
	It's only mean to implement material pointer and 
*/
struct primitive
{
	const abstract_material *material = nullptr;
};

/**
	A sphere defined by its origin and radius.

	\warning Sphere transformations aren't quite perfect - radius is only affected by
	scaling in the X axis. Hence the spheres should only be scaled uniformly.
*/
struct sphere : public primitive
{
	sphere(const glm::vec3 &o, float r) :
		origin(o),
		radius(r)
	{}

	static inline const sphere *ray_intersect(const sphere *begin, const sphere *end, const ray &r, ray_intersection &isec);
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const;
	inline ray_hit get_ray_hit(const ray_intersection &isec, const ray &r) const;
	inline aabb get_aabb() const;
	inline sphere transform(const glm::mat4 &mat) const;
	
	glm::vec3 origin;
	float radius;
};

/**
	Checks spheres for intersection with a ray

	\note This function accounts for distance already stored in isec parameter! Farther intersections
		won't be reported.
	
	\returns pointer to the nearset intersected sphere, otherwise nullptr
*/
inline const sphere *sphere::ray_intersect(const sphere *begin, const sphere *end, const ray &r, ray_intersection &isec)
{
	const sphere *best_p = nullptr;

	for (const sphere *p = begin; p != end; p++)
	{
		// t^2 * d * d + t * 2d(o-c) + (o-c)(o-c) - R^2 = 0
		glm::vec3 u = r.origin - p->origin;
		float a = glm::dot(r.direction, r.direction);
		float b = 2 * glm::dot(r.direction, u);
		float c = glm::dot(u, u) - p->radius * p->radius;
		float delta = b * b - 4 * a * c;

		if (delta < 0) continue;
		else
		{
			// Check whether the ray is cast from inside of the sphere
			// (if roots have the same sign)
			if (c * a > 0)
			{
				// Only return the closer intersection
				float t = (-b - std::sqrt(delta)) / (2 * a);

				// Reject negative distance (both roots have the same sign)
				// and too distant intersections
				if (t < 0 || t > isec.distance)
					continue;

				isec.distance = t;
				best_p = p;
			}
			else
			{
				// We only care about the positive (greater) root
				float t = (-b + std::sqrt(delta)) / (2 * a);
				if (t > isec.distance)
					continue;

				isec.distance = t;
				best_p = p;
			}
		}
	}

	return best_p;
}

bool sphere::ray_intersect(const ray &r, ray_intersection &isec) const
{
	isec.distance = rt::ray_miss;
	return sphere::ray_intersect(this, this + 1, r, isec);
}

inline ray_hit sphere::get_ray_hit(const ray_intersection &isec, const ray &r) const
{
	ray_hit h;
	h.distance = isec.distance;
	h.direction = r.direction;
	h.position = r.origin + isec.distance * r.direction;
	h.normal = glm::normalize(h.position - this->origin);
	h.material = this->material;
	return h;
}

inline aabb sphere::get_aabb() const
{
	return aabb{origin - glm::vec3{radius}, origin + glm::vec3{radius}};
}

/**
	Returns a new sphere resulting from matrix transformation
*/
inline sphere sphere::transform(const glm::mat4 &mat) const
{
	rt::sphere p(*this);

	glm::vec3 x = this->origin + glm::vec3{this->radius, 0.f, 0.f};
	x = mat * glm::vec4{x, 1.f};
	p.origin = mat * glm::vec4{this->origin, 1.f};
	p.radius = glm::length(x - p.origin);

	return p;
}

/**
	Infinite plane defined by origin point and normal vector
*/
struct plane : public primitive
{
	plane(const glm::vec3 &o, const glm::vec3 &n) :
		origin(o),
		normal(n)
	{}

	static inline const plane *ray_intersect(const plane *begin, const plane *end, const ray &r, ray_intersection &isec);
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const;
	inline ray_hit get_ray_hit(const ray_intersection &isec, const ray &r) const;
	inline aabb get_aabb() const;
	inline plane transform(const glm::mat4 &mat) const;

	glm::vec3 origin;
	glm::vec3 normal;
};

/**
	Checks planes for intersection with a ray

	\note This function accounts for distance already stored in isec parameter! Farther intersections
		won't be reported.
	
	\returns pointer to the nearset intersected plane, otherwise nullptr
*/
inline const plane *plane::ray_intersect(const plane *begin, const plane *end, const ray &r, ray_intersection &isec)
{
	const rt::plane *best_p = nullptr;

	for (const rt::plane *p = begin; p != end; p++)
	{
		float n_dot_dir = glm::dot(p->normal, r.direction);
		if (n_dot_dir != 0)
		{
			float t = glm::dot(p->normal, p->origin - r.origin) / n_dot_dir;

			// Reject negative and too distant intersections
			if (t < 0 || t > isec.distance)
				continue;

			isec.distance = t;
			best_p = p;
		}
	}

	return best_p;
}

bool plane::ray_intersect(const ray &r, ray_intersection &isec) const
{
	isec.distance = rt::ray_miss;
	return plane::ray_intersect(this, this + 1, r, isec);
}

/**
	Computes ray_hit from ray_intersection
*/
inline ray_hit plane::get_ray_hit(const ray_intersection &isec, const ray &r) const
{
	ray_hit h;
	h.distance = isec.distance;
	h.direction = r.direction;
	h.position = r.origin + isec.distance * r.direction;
	h.normal = this->normal;
	h.material = this->material;
	return h;
}

/**
	Returns AABB with -HUGE_VALFs and +HUGE_VALFs
*/
inline aabb plane::get_aabb() const
{
	return {{-HUGE_VALF, -HUGE_VALF, -HUGE_VALF}, {HUGE_VALF, HUGE_VALF, HUGE_VALF}};
}

/**
	Returns plane resulting from matrix transform
*/
inline plane plane::transform(const glm::mat4 &mat) const
{
	plane p{*this};

	p.origin = mat * glm::vec4{this->origin, 1.f};
	p.normal = mat * glm::vec4{this->normal, 0.f};

	return p;
}

/**
	A triangle primitive defined by 3 vertices, their normals and UVs
*/
struct triangle : public primitive
{
	static inline const triangle *ray_intersect(const triangle *begin, const triangle *end, const ray &r, ray_intersection &isec);
	inline bool ray_intersect(const ray &r, ray_intersection &isec) const;
	inline ray_hit get_ray_hit(const ray_intersection &isec, const ray &r) const;
	inline aabb get_aabb() const;
	inline triangle transform(const glm::mat4 &mat) const;

	glm::vec3 vertices[3];
	glm::vec3 normals[3];
	glm::vec2 uvs[3];
};

/**
	Based on: https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf

	\note This function accounts for distance already stored in isec parameter! Farther intersections
		won't be reported.
	
	\returns pointer to the nearset intersected triangle, otherwise nullptr
*/
inline const triangle *triangle::ray_intersect(const triangle *begin, const triangle *end, const ray &r, ray_intersection &isec)
{
	const triangle *best_p = nullptr;

	for (const triangle *p = begin; p != end; p++)
	{
		glm::vec3 E1 = p->vertices[1] - p->vertices[0];
		glm::vec3 E2 = p->vertices[2] - p->vertices[0];
		glm::vec3 P = glm::cross(r.direction, E2);
		float det = glm::dot(P, E1);
		if (det == 0.0)
			continue;

		//! \todo Backface culling

		glm::vec3 T = r.origin - p->vertices[0];
		glm::vec3 Q = glm::cross(T, E1);
		glm::vec3 tuv = glm::vec3{glm::dot(Q, E2), glm::dot(P, T), glm::dot(Q, r.direction)} * (1.f/ det);
		float t = tuv.x;
		float u = tuv.y;
		float v = tuv.z;

		// Check u bounds
		if (u < 0.f || u > 1.f)
			continue;

		// Check v bounds
		if (v < 0.f || u + v > 1.f)	
			continue;

		// Reject negative and too large t
		if (t < 0.f || t > isec.distance)
			continue;

		isec.distance = t;
		isec.u = u;
		isec.v = v;
		best_p = p;
	}

	return best_p;
}

bool triangle::ray_intersect(const ray &r, ray_intersection &isec) const
{
	isec.distance = rt::ray_miss;
	return triangle::ray_intersect(this, this + 1, r, isec);
}

/**
	Build ray_hit based on ray_intersection. Normal vector calculation
	is performed here.
*/
inline ray_hit triangle::get_ray_hit(const ray_intersection &isec, const ray &r) const
{
	ray_hit h;
	h.distance = isec.distance;
	h.direction = r.direction;
	h.position = r.origin + isec.distance * r.direction;
	h.normal = glm::normalize(this->normals[0] * (1 - isec.u - isec.v) + this->normals[1] * isec.u + this->normals[2] * isec.v);
	h.material = this->material;
	return h;
}

/**
	Calculates AABB by taking min and max values of all vertices' coordinates
*/
inline aabb triangle::get_aabb() const
{
	glm::vec3 min{glm::min(vertices[0], glm::min(vertices[1], vertices[2]))};
	glm::vec3 max{glm::max(vertices[0], glm::max(vertices[1], vertices[2]))};
	return aabb{min, max};
}

/**
	Returns triangle resulting from matrix transfomation
*/
inline triangle triangle::transform(const glm::mat4 &mat) const
{
	triangle t{*this};

	t.vertices[0] = mat * glm::vec4{this->vertices[0], 1.f};
	t.vertices[1] = mat * glm::vec4{this->vertices[1], 1.f};
	t.vertices[2] = mat * glm::vec4{this->vertices[2], 1.f};

	t.normals[0] = mat * glm::vec4{this->normals[0], 0.f};
	t.normals[1] = mat * glm::vec4{this->normals[1], 0.f};
	t.normals[2] = mat * glm::vec4{this->normals[2], 0.f};

	t.uvs[0] = this->uvs[0];
	t.uvs[1] = this->uvs[1];
	t.uvs[2] = this->uvs[2];

	return t;
}

}