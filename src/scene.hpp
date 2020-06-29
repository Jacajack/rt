#pragma once
#include <vector>
#include <memory>
#include "ray.hpp"
#include "material.hpp"

namespace rt {

/**
	Anything that can be put on the scene
*/
class scene_object
{
public:
	scene_object(ray_intersectable &geometry, material &mat) :
		m_geometry(&geometry),
		m_material(&mat)
	{}

	// TEMP
	const ray_intersectable &get_geometry() const
	{
		return *m_geometry;
	}

	const material &get_material() const
	{
		return *m_material;
	}

private:
	ray_intersectable *m_geometry; //! \todo replace with pretransformed geometry
	material *m_material;
};

/**
	A simple sky material that can be used by the scene
*/
class simple_sky_material : public material
{
public:
	simple_sky_material() :
		material(true, false)
	{}

	glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const override
	{
		return glm::vec3{0.7, 0.7, 1.0} * (1 - std::abs(V.y)) + glm::vec3{0.1, 0.2, 0.4} * std::abs(V.z);
	}
};

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
	std::unique_ptr<material> m_world_material = std::make_unique<simple_sky_material>();
};

}