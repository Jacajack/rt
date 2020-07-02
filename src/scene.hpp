#pragma once
#include <vector>
#include <memory>
#include "ray.hpp"
#include "material.hpp"
#include "ray_accelerator.hpp"

namespace rt {

/**
	Anything that can be put on the scene
*/
class scene_object
{
public:
	scene_object(ray_intersectable &geometry, abstract_material &mat) :
		m_geometry(&geometry),
		m_material(&mat)
	{}

	// TEMP
	const ray_intersectable &get_geometry() const
	{
		return *m_geometry;
	}

	const abstract_material &get_material() const
	{
		return *m_material;
	}

private:
	ray_intersectable *m_geometry; //! \todo replace with pretransformed geometry
	abstract_material *m_material;
};

/**
	A simple sky material that can be used by the scene
*/
class simple_sky_material : public abstract_material
{
public:

	rt::ray_bounce get_bounce(const ray_hit &hit, float r1, float r2) const override
	{
		rt::ray_bounce bounce;
		bounce.brdf = glm::vec3{0.f};
		bounce.reflection_pdf = 1.f;
		bounce.btdf = glm::vec3{0.f};
		// bounce.emission = glm::vec3{0.5, 0.5, 1.0} + 10.f * glm::vec3{std::pow(glm::dot(hit.direction, {0, 1, 0}), 4.f)}; 
		bounce.emission = glm::mix(glm::vec3{0.3, 0.3, 0.4}, glm::vec3{1.1, 1.0, 2.0}, std::abs(hit.direction.y));
		return bounce;
	}
};

/**
	Contains entities
*/
class scene
{
	friend class primitive_soup;

public:
	void add_object(scene_object *obj);
	
	ray_hit cast_ray(const ray &r) const;
	ray_hit cast_ray(const ray &r, const ray_accelerator &accel) const;

private:
	std::vector<scene_object*> m_objects;
	std::unique_ptr<abstract_material> m_world_material = std::make_unique<simple_sky_material>();
};

}