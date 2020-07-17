#pragma once
#include <vector>
#include <memory>
#include "ray.hpp"
#include "camera.hpp"
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
	scene_object(const std::shared_ptr<const primitive_collection> &primitives) :
		m_primitives(primitives)
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
		col.apply_transform(m_transform);
		return col;
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
	std::shared_ptr<const primitive_collection> m_primitives;
};

/**
	Contains entities
*/
class scene
{
public:
	void add_object(const std::shared_ptr<rt::scene_object> &obj)
	{
		m_objects.push_back(obj);
	}

	void add_material(const std::shared_ptr<rt::abstract_material> &mat)
	{
		m_materials.push_back(mat);
	}

	const std::vector<std::shared_ptr<rt::scene_object>> &get_objects() const
	{
		return m_objects;
	}
	
	ray_hit cast_ray(const ray &r, const ray_accelerator &accel) const;

	void set_camera(const std::shared_ptr<rt::camera> &c)
	{
		m_camera = c;
	}

	rt::camera &get_camera()
	{
		return *m_camera;
	}

	const rt::camera &get_camera() const
	{
		return *m_camera;
	}

	template <typename T>
	void init_accelerator()
	{
		m_accelerator_ptr = std::make_unique<T>(*this);
	}

	void set_accelerator(std::unique_ptr<rt::ray_accelerator> ptr)
	{
		m_accelerator_ptr = std::move(ptr);
	}

	const rt::ray_accelerator &get_accelerator() const
	{
		return *m_accelerator_ptr;
	}

private:
	std::vector<std::shared_ptr<rt::scene_object>> m_objects;
	std::vector<std::shared_ptr<rt::abstract_material>> m_materials;

	std::unique_ptr<abstract_material> m_world_material = std::make_unique<simple_sky_material>();

	std::shared_ptr<rt::camera> m_camera;
	std::unique_ptr<rt::ray_accelerator> m_accelerator_ptr;
};

}