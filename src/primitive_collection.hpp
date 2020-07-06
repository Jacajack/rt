#pragma once

#include "primitive.hpp"
#include "aabb.hpp"
#include "mesh_data.hpp"

#include <vector>

namespace rt {

/**
	Stores primitives of all kinds. Can collectively
	apply transforms and set/assign materials.
*/
struct primitive_collection
{
	explicit primitive_collection(const mesh_data &mesh);
	explicit primitive_collection(mesh_data &&mesh);
	explicit primitive_collection(const triangle &t);
	explicit primitive_collection(const sphere &s);
	explicit primitive_collection(const plane &p);

	/**
		Applies matrix transform to all primitives that currently are in this collection.
	*/
	void apply_transform(const glm::mat4 &mat);

	/**
		Assigns material to all primitives that currently are in this collection and
		have no material assigned.
	*/
	void assign_material(const abstract_material *material);

	/**
		Assigns material to all primitives that currently are in this collection
	*/
	void set_material(const abstract_material *material);

	std::vector<rt::triangle> triangles;
	std::vector<rt::sphere> spheres;
	std::vector<rt::plane> planes;
};

}