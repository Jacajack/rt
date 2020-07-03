#pragma once

#include "linear_tree.hpp"
#include "ray_accelerator.hpp"
#include "primitive_soup.hpp"
#include "aabb.hpp"

namespace rt {

/**
	A node of a BVH tree
*/
struct bvh_tree_node
{
	/**
		Adds triangles range to the node and calculates its bounding volume
	*/
	bvh_tree_node(soup_triangle *begin, soup_triangle *end);

	aabb bounding_volume;
	bool children_overlap;
	soup_triangle *begin;
	soup_triangle *end;
};

/**
	A more efficient implementation of a BVH tree
*/
class bvh_tree : public ray_accelerator
{
public:
	bvh_tree(const primitive_soup &soup);
	bool cast_ray(const rt::ray &r, ray_hit &hit) const override;

private:
	struct node_intersection
	{
		node_intersection() = default;

		node_intersection(linear_tree<bvh_tree_node>::iterator it, float u) :
			node(it),
			t(u)
		{}

		linear_tree<bvh_tree_node>::iterator node;
		float t;
	};

	void build_tree();

	linear_tree<bvh_tree_node> m_tree;
	std::vector<soup_triangle> m_triangles;
	std::vector<soup_sphere> m_spheres;
	std::vector<soup_plane> m_planes;
};

}