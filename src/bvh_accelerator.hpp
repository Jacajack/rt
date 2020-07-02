#pragma once

#include <vector>
#include <memory>

#include "primitive_soup.hpp"
#include "ray_accelerator.hpp"
#include "aabb.hpp"

namespace rt {

/**
	BVH node can be created from primitive soup. The tree structure
	is achieved by recursively subdividing the node with subdivide()
*/
class bvh_node
{
public:
	template <typename T>
	bvh_node(T begin, T end);

	bvh_node(const primitive_soup &soup);
	bvh_node(primitive_soup &&soup);

	//! Splits owned primitives between children
	void subdivide();

	/**
		Recursively splits the node until all leaves contain less than
		certain number of primitives.
	*/
	void recursively_subdivide(unsigned int min_primitve_count = 4);

	/**
		Recursively searches children nodes for intersection with provided ray
	*/
	bool cast_ray(const rt::ray &r, rt::ray_hit &hit) const;

	const aabb &get_aabb() const
	{
		return m_volume;
	}

private:
	void calculate_volume();

	aabb m_volume;
	std::unique_ptr<bvh_node> m_left;
	std::unique_ptr<bvh_node> m_right;
	bool m_children_overlap = false;
	std::vector<rt::soup_triangle> m_triangles;
};

template <typename T>
bvh_node::bvh_node(T begin, T end)
{
	std::copy(begin, end, std::back_inserter(m_triangles));
	calculate_volume();
}

/**
	Bounding Volume Hierarchy ray accelerator. This a proof-of-concept implementation
	and probably won't very efficient.

	\warning This implementation only handles triangles. Sphere and plane intersections
		are found with linear search.
*/
class bvh_accelerator : public ray_accelerator
{
public:
	bvh_accelerator(const primitive_soup &soup);
	bvh_accelerator(primitive_soup &&soup);

	bool cast_ray(const rt::ray &r, ray_hit &hit) const override;

private:
	void build_tree();

	bvh_node m_root_node;
	std::vector<soup_sphere> m_spheres;
	std::vector<soup_plane> m_planes;
};

}