#pragma once
#include <vector>
#include "ray.hpp"
#include "material.hpp"

namespace rt {

/**
	Anything that can be put on the scene
*/
class scene_object
{
public:
	scene_object(ray_intersectable &geometry) :
		m_geometry(&geometry),
		m_material(nullptr)
	{}

	/**
		Intersects the ray with object's geometry
	*/
	inline bool cast_ray(const ray &r, ray_hit &hit) const;

private:
	ray_intersectable *m_geometry;
	material *m_material;
};

/**
	Evaluates ray intersection with the object and provides BRDF for the intersection point
*/
bool scene_object::cast_ray(const ray &r, ray_hit &hit) const
{
	static diffuse_brdf test_brdf{glm::vec3{0.5, 0, 0}};

	if (m_geometry->ray_intersect(r, hit))
	{
		hit.brdf = &test_brdf;
		return true;
	}

	return false;
}

/**
	Contains entities
*/
class scene
{
public:
	void add_object(scene_object *obj);
	
	ray_hit cast_ray(const ray &r) const;

private:
	std::vector<scene_object*> m_objects;
};

}