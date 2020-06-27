#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"

namespace rt {

/**
	Abstract base for providing ray_intersect functionality
*/
struct ray_intersectable
{
	virtual inline bool ray_intersect(const ray &r, ray_intersection &hit) const = 0;
};

struct sphere : public ray_intersectable
{
	sphere(const glm::vec3 &o, float r) :
		origin(o),
		radius(r)
	{}

	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;

	glm::vec3 origin;
	float radius;
};

bool sphere::ray_intersect(const ray &r, ray_intersection &hit) const
{
	// t^2 * d * d + t * 2d(o-c) + (o-c)(o-c) - R^2 = 0
	glm::vec3 u = r.origin - this->origin;

	float a = glm::dot(r.direction, r.direction);
	float b = 2 * glm::dot(r.direction, u);
	float c = glm::dot(u, u) - this->radius * this->radius;
	float delta = b * b - 4 * a * c;

	if (delta < 0) return false;
	else
	{
		// Check whether the ray is cast from inside of the sphere
		// (if roots have the same sign)
		if (c * a > 0)
		{
			// Only return the closer intersection
			float t = (-b - std::sqrt(delta)) / (2 * a);

			// Reject negative distance (both roots have the same sign)
			if (t < 0) return false;

			hit.distance = t;
			hit.direction = r.direction;
			hit.position = r.origin + r.direction * t;
			hit.normal = glm::normalize(hit.position - this->origin);
			return true;
		}
		else
		{
			// We only care about the positive (greater) root
			float t = (-b + std::sqrt(delta)) / (2 * a);
			hit.distance = t;
			hit.direction = r.direction;
			hit.position = r.origin + r.direction * t;
			hit.normal = glm::normalize(hit.position - this->origin);
			return true;
		}
	}
}

struct plane : public ray_intersectable
{
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;

	glm::vec3 origin;
	glm::vec3 normal;
};

bool plane::ray_intersect(const ray &r, ray_intersection &hit) const
{
	float n_dot_dir = glm::dot(this->normal, r.direction);
	if (n_dot_dir != 0)
	{
		float t = glm::dot(this->normal, this->origin - r.origin) / n_dot_dir;

		hit.distance = t;
		hit.direction = r.direction;
		hit.position = r.origin + r.direction * t;
		hit.normal = this->normal;

		return true;
	}

	// Miss
	return false;
}

struct triangle : public ray_intersectable
{
	glm::vec3 vertices[3];
	glm::vec3 normals[3];
};

struct triangle_mesh : public ray_intersectable
{
};

}