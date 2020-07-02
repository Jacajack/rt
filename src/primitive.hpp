#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"

namespace rt {

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
	plane(const glm::vec3 &o, const glm::vec3 &n) :
		origin(o),
		normal(n)
	{}

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

		// Reject negative
		if (t < 0) return false;

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
	glm::vec2 uvs[3];

	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;
};

/**
	Based on: https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
*/
bool triangle::ray_intersect(const ray &r, ray_intersection &hit) const
{
	glm::vec3 E1 = vertices[1] - vertices[0];
	glm::vec3 E2 = vertices[2] - vertices[0];
	glm::vec3 P = glm::cross(r.direction, E2);
	float det = glm::dot(P, E1);
	if (det == 0.0) return false;
	float inv_det = 1.f / det;

	//! \todo Backface culling
	//! \todo Check if calculating vector components one by one increases perf

	glm::vec3 T = r.origin - vertices[0];
	glm::vec3 Q = glm::cross(T, E1);
	glm::vec3 tuv = glm::vec3{glm::dot(Q, E2), glm::dot(P, T), glm::dot(Q, r.direction)} * (1.f/ det);
	float t = tuv.x;
	float u = tuv.y;
	float v = tuv.z;

	// Check u bounds
	if (u < 0.f || u > 1.f)
		return false;

	// Check v bounds
	if (v < 0.f || u + v > 1.f)	
		return false;

	// Reject negative t
	if (t < 0.f)
		return false;

	// Return hit
	hit.distance = t;
	hit.direction = r.direction;
	hit.position = r.origin + t * r.direction;
	hit.normal = glm::normalize(normals[0] * (1.f - u - v) + normals[1] * u + normals[2] * v);
	return true;
}

}