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
	ray_intersection + pointers to material, geometry and object
*/
struct ray_hit : public ray_intersection
{
	const ray_intersectable *geometry;
	const material *mat;
	const scene_object *object;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
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