#include "bvh_tree.hpp"

#include <stack>
#include <queue>

#include "ray.hpp"
#include "linear_stack.hpp"

using rt::bvh_tree;
using rt::bvh_tree_node;

bvh_tree_node::bvh_tree_node(soup_triangle *b, soup_triangle *e) :
	children_overlap(true),
	begin(b),
	end(e)
{
	aabb vol({0, 0, 0}, {0, 0, 0});
	for (auto p = b; p && p != e; p++)
		vol = aabb{vol, p->get_aabb()};
	bounding_volume = vol;
}

bvh_tree::bvh_tree(const rt::primitive_soup &soup) :
	m_tree(14),
	m_triangles(soup.triangles),
	m_spheres(soup.spheres),
	m_planes(soup.planes)
{
	m_tree.get_root_node().emplace(m_triangles.data(), m_triangles.data() + m_triangles.size());
	build_tree();
}

void bvh_tree::build_tree()
{
	std::stack<linear_tree<bvh_tree_node>::iterator> to_process;
	to_process.push(m_tree.get_root_node());	

	while (!to_process.empty())
	{
		auto it = to_process.top();
		to_process.pop();

		// If the node has less than N triangles, do not subdivide it
		if (it->end - it->begin <= 32)
			continue;

		// Get center and size of the node's bounding volume
		glm::vec3 volsize{it->bounding_volume.get_size()};
		glm::vec3 volcenter{it->bounding_volume.get_center()};

		// Decide split direction
		float glm::vec3::* axis;
		if (volsize.x > volsize.y && volsize.x > volsize.z)
			axis = &glm::vec3::x;
		else if (volsize.y > volsize.x && volsize.y > volsize.z)
			axis = &glm::vec3::y;
		else
			axis = &glm::vec3::z;

		// Sort the triangles based on the selected component
		// \todo Do not calculate AABBs over and over!!! Store triangles along with their centroids!
		std::sort(it->begin, it->end, [&](auto &t1, auto &t2){
			glm::vec3 c1 = (t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f;
			glm::vec3 c2 = (t2.vertices[0] + t2.vertices[1] + t2.vertices[2]) / 3.f;
			return c1.*axis < c2.*axis;
		});

		// Attempt midpoint split
		auto split = std::lower_bound(it->begin, it->end, volcenter.*axis, [&](auto &t1, auto c){
			return ((t1.vertices[0] + t1.vertices[1] + t1.vertices[2]) / 3.f).*axis < c;
		});

		// If any child is empty, attempt median split
		if (split == it->end || split == it->begin)
			split = it->begin + (it->end - it->begin) / 2;
		
		// Create children and remove triangles from this node
		it.left().emplace(it->begin, split);
		it.right().emplace(split, it->end);
		it->begin = nullptr;
		it->end = nullptr;

		// Check if the children overlap
		it->children_overlap = it.left()->bounding_volume.check_aabb_overlap(it.right()->bounding_volume);

		// Add children to the processing stack
		to_process.push(it.left());
		to_process.push(it.right());
	}
}

bool bvh_tree::cast_ray(const rt::ray &r, ray_hit &best_hit) const
{
	rt::ray_hit h;
	best_hit.distance = HUGE_VALF;

	// Check spheres
	for (const auto &s : m_spheres)
		if (s.cast_ray(r, h))
			best_hit = std::min(best_hit, h);

	// Check planes
	for (const auto &p : m_planes)
		if (p.cast_ray(r, h))
			best_hit = std::min(best_hit, h);


	// Intersections with tree nodes
	rt::linear_stack<node_intersection, 256> intersections;

	// Check intersection with the root node
	auto root = m_tree.get_root_node();
	if (root.has_value())
	{
		float t = root->bounding_volume.ray_intersection_distance(r);
		if (t != HUGE_VALF)
			intersections.emplace(root, t);
	}

	// Process all intersections
	while (!intersections.empty())
	{
		auto isec = intersections.top();
		intersections.pop();
		auto node = isec.node;
		float t = isec.t;

		// If an intersection was found earlier, closer than this volume itself - skip
		if (best_hit.distance < t) continue;

		// If this is a leaf node, intersect all triangles
		// and do not traverse further
		if (node->begin != node->end)
		{
			for (auto p = node->begin; p != node->end; p++)
				if (p->cast_ray(r, h))
					best_hit = std::min(best_hit, h);

			continue;
		}

		// Compute child nodes intersections
		float tl = node.left()->bounding_volume.ray_intersection_distance(r);
		float tr = node.right()->bounding_volume.ray_intersection_distance(r);

		if (tl == HUGE_VALF && tr == HUGE_VALF)
			continue;
		else if (tl == HUGE_VALF || tr == HUGE_VALF)
		{
			// Missed only one child
			if (tr == HUGE_VALF && tl < best_hit.distance) intersections.emplace(node.left(), tl);
			else if (tl == HUGE_VALF && tr < best_hit.distance) intersections.emplace(node.right(), tr);
		}
		else
		{
			// Hit both
			// If the children overlap, put the farther child on the stack first
			// Otherwise only check the closer one
			if (node->children_overlap)
			{
				if (tl > tr)
				{
					if (tl < best_hit.distance)
						intersections.emplace(node.left(), tl);
					
					intersections.emplace(node.right(), tr);
				}
				else
				{
					if (tr < best_hit.distance)
						intersections.emplace(node.right(), tr);

					intersections.emplace(node.left(), tl);
				}
			}
			else // Only check the closer one
			{
				if (tl > tr)
					intersections.emplace(node.left(), tl);
				else
					intersections.emplace(node.right(), tr);
			}
		}
	}

	return best_hit.distance != HUGE_VALF;
}
