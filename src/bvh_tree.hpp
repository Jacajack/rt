#pragma once

#include "linear_tree.hpp"
#include "ray_accelerator.hpp"
#include "primitive.hpp"
#include "scene.hpp"

namespace rt {

/**
	A node of a BVH tree
*/
struct bvh_tree_node
{
	/**
		Adds triangles range to the node and calculates its bounding volume
	*/
	bvh_tree_node(triangle *begin, triangle *end);

	rt::aabb bounding_volume;
	rt::triangle *begin;
	rt::triangle *end;
};

/**
	A more efficient implementation of a BVH tree
*/
class bvh_tree : public ray_accelerator
{
public:
	bvh_tree(const scene &scene);
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
	std::vector<triangle> m_triangles;
	std::vector<sphere> m_spheres;
	std::vector<plane> m_planes;
};

}