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