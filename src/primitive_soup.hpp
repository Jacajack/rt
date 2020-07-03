#pragma once

#include <vector>
#include "primitive.hpp"
#include "scene.hpp"

namespace rt {

/**
	All primitives in primitive_soup must contain pointers to the objects they were
	extracted from
*/
struct soup_primitive
{
	soup_primitive(const scene_object *obj) :
		object(obj)
	{}

	const scene_object *object;
};

struct soup_triangle : public triangle, public soup_primitive
{
	soup_triangle(const triangle &t, const scene_object *obj) :
		triangle(t),
		soup_primitive(obj)
	{}

	inline bool cast_ray(const rt::ray &r, rt::ray_hit &hit) const
	{
		rt::ray_intersection isec;
		if (triangle::ray_intersect(r, isec))
		{
			hit.position = isec.position;
			hit.direction = isec.direction;
			hit.distance = isec.distance;
			hit.normal = isec.normal;
			hit.material = &object->get_material();
			hit.object = object;
			return true;
		}

		return false;
	}

	/**
		Checks ray against many triangles
	*/
	static inline ray_hit intersect_triangles(const soup_triangle *begin, const soup_triangle *end, const ray &r, float start_t = HUGE_VALF)
	{
		bool did_hit = false;
		float best_t = start_t;
		float best_u, best_v;
		const soup_triangle *best_p = nullptr;

		for (const soup_triangle *p = begin; p != end; p++)
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
			if (t < 0.f || t > best_t)
				continue;

			best_t = t;
			best_u = u;
			best_v = v;
			best_p = p;
			did_hit |= true;		
		}

		// Return hit
		rt::ray_hit hit;
		if (did_hit)
		{
			hit.distance = best_t;
			hit.direction = r.direction;
			hit.position = r.origin + best_t * r.direction;
			hit.normal = glm::normalize(best_p->normals[0] * (1.f - best_u - best_v) + best_p->normals[1] * best_u + best_p->normals[2] * best_v);
			hit.material = &best_p->object->get_material();
			hit.object = best_p->object;
		}
		else
		{
			hit.distance = HUGE_VALF;
		}
		return hit;
	}
};

struct soup_sphere : public sphere, public soup_primitive
{
	soup_sphere(const sphere &s, const scene_object *obj) :
		sphere(s),
		soup_primitive(obj)
	{}

	inline bool cast_ray(const rt::ray &r, rt::ray_hit &hit) const
	{
		rt::ray_intersection isec;
		if (sphere::ray_intersect(r, isec))
		{
			hit.position = isec.position;
			hit.direction = isec.direction;
			hit.distance = isec.distance;
			hit.normal = isec.normal;
			hit.material = &object->get_material();
			hit.object = object;
			return true;
		}

		return false;
	}
};

struct soup_plane : public plane, public soup_primitive
{
	soup_plane(const plane &p, const scene_object *obj) :
		plane(p),
		soup_primitive(obj)
	{}

	inline bool cast_ray(const rt::ray &r, rt::ray_hit &hit) const
	{
		rt::ray_intersection isec;
		if (plane::ray_intersect(r, isec))
		{
			hit.position = isec.position;
			hit.direction = isec.direction;
			hit.distance = isec.distance;
			hit.normal = isec.normal;
			hit.material = &object->get_material();
			hit.object = object;
			return true;
		}

		return false;
	}
};

/**
	Contains individual primitives (spheres, triangles, planes) with all transforms applied.
	Basically primitive_soup is a decomposed version of the scene.

	Primitive soup is created from a scene and then passed to some kind of ray_accelerator.
*/
struct primitive_soup
{
	primitive_soup(const rt::scene &sc);

	std::vector<soup_triangle> triangles;
	std::vector<soup_plane> planes;
	std::vector<soup_sphere> spheres;
};

}