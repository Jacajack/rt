#pragma once
#include <vector>
#include <memory>
#include "ray.hpp"
#include "material.hpp"
#include "ray_accelerator.hpp"
#include "primitive_collection.hpp"

#include "materials/simple_sky.hpp"

namespace rt {

/**
	Anything that can be put on the scene
*/
class scene_object
{
public:
	scene_object(const primitive_collection &primitives, const abstract_material &mat) :
		m_material(&mat),
		m_primitives(&primitives)
	{}

	const primitive_collection &get_primitives() const
	{
		return *m_primitives;
	}

	/**
		Returns primitive collection with assigned material and transforms applied
	*/
	primitive_collection get_transformed_primitive_collection() const
	{
		if (!m_primitives) throw std::runtime_error("object has empty primitive collection");

		primitive_collection col{*m_primitives};
		col.assign_material(m_material);
		col.apply_transform(m_transform);
		return col;
	}

	/**
		Assigns material to the entire object
	*/
	void set_material(const abstract_material *material)
	{
		m_material = m_material;
	}

	/**
		Returns currently used material
	*/
	const abstract_material &get_material() const
	{
		return *m_material;
	}

	/**
		Sets model matrix
	*/
	void set_transform(const glm::mat4 &mat)
	{
		m_transform = mat;
	}

	/**
		Returns currenlty used model matrix
	*/
	const glm::mat4 &get_transform() const
	{
		return m_transform;
	}

private:
	glm::mat4 m_transform = glm::mat4(1.f);
	const abstract_material *m_material;
	const primitive_collection *m_primitives = nullptr;
};

/**
	Contains entities
*/
class scene
{
public:
	void add_object(scene_object *obj);
	
	const std::vector<scene_object*> &get_objects() const
	{
		return m_objects;
	}
	
	ray_hit cast_ray(const ray &r, const ray_accelerator &accel) const;

private:
	std::vector<scene_object*> m_objects;
	std::unique_ptr<abstract_material> m_world_material = std::make_unique<simple_sky_material>();
};

}